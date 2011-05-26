#include "BitFile.h"
#include "BitData.h"
#include "BitPiece.h"
#include "../base/ScopePtr.h"
#include "../base/StringConv.h"
#include "../thread/Atomic.h"
#include "../thread/Thread.h"
#include "../thread/Event.h"
#include "../thread/Mutex.h"
#include <algorithm>
#include <functional>
#include <string>
#include <iterator>
#include <shlwapi.h>

using namespace std::tr1::placeholders;

namespace bitwave {
namespace core {

    class BitFile::FileService : private NotCopyable
    {
        class File : private NotCopyable
        {
        public:
            File(const std::string& path, long long length, bool download)
                : path_(path),
                  length_(length),
                  download_(download),
                  file_handle_(INVALID_HANDLE_VALUE)
            {
                if (download_)
                    OpenFile();
            }

            ~File()
            {
                if (!Invalidate())
                    ::CloseHandle(file_handle_);
            }

            void Read(long long file_pos, long long read_bytes, char *buffer)
            {
                if (Invalidate())
                    return ;
                SeekToPos(file_pos);
                unsigned long read = 0;
                ::ReadFile(file_handle_, buffer,
                        static_cast<unsigned long>(read_bytes), &read, 0);
            }

            void Write(long long file_pos, long long write_bytes, const char *buffer)
            {
                if (Invalidate())
                    return ;
                SeekToPos(file_pos);
                unsigned long writed = 0;
                ::WriteFile(file_handle_, buffer,
                        static_cast<unsigned long>(write_bytes), &writed, 0);
            }

            void Flush()
            {
                if (Invalidate())
                    return ;
                ::FlushFileBuffers(file_handle_);
            }

        private:
            void OpenFile()
            {
                std::wstring path = UTF8ToUnicode(path_);
                CreateFileDirectory(path);

                file_handle_ = ::CreateFile(path.c_str(),
                        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, 0);

                if (file_handle_ == INVALID_HANDLE_VALUE)
                    throw CreateFileError(CreateFileError::PATH_ERROR);

                if (!SeekToPos(length_) || !::SetEndOfFile(file_handle_))
                    throw CreateFileError(CreateFileError::SPACE_NOT_ENOUGH);
            }

            void CreateFileDirectory(const std::wstring& path)
            {
                std::wstring::size_type pos = path.find_last_of('\\');
                if (pos == std::wstring::npos)
                    throw CreateFileError(CreateFileError::PATH_ERROR);

                CreateDir(path.substr(0, pos + 1));
            }

            void CreateDir(const std::wstring& path)
            {
                std::wstring::size_type pos = path.find('\\');
                while (pos != std::wstring::npos)
                {
                    std::wstring sub_dir = path.substr(0, pos + 1);
                    if (!::PathFileExists(sub_dir.c_str()) &&
                            !::CreateDirectory(sub_dir.c_str(), 0))
                        throw CreateFileError(CreateFileError::PATH_ERROR);
                    pos = path.find('\\', pos + 1);
                }
            }

            bool SeekToPos(long long file_pos)
            {
                LARGE_INTEGER file_ptr;
                file_ptr.QuadPart = file_pos;
                return ::SetFilePointerEx(file_handle_, file_ptr, 0, FILE_BEGIN) != 0;
            }

            bool Invalidate() const
            {
                return file_handle_ == INVALID_HANDLE_VALUE;
            }

            std::string path_;
            long long length_;
            bool download_;
            HANDLE file_handle_;
        };

    public:
        explicit FileService(const std::tr1::shared_ptr<BitData>& bitdata)
            : piece_length_(bitdata->GetPieceLength()),
              thread_exit_flag_(0)
        {
            PrepareFiles(bitdata);
            PrepareIoThread();
        }

        ~FileService()
        {
            AtomicAdd(&thread_exit_flag_, 1);
            ops_event_.SetEvent();
            io_thread_->Join();
        }

        void ReadPiece(std::size_t piece_index, const PiecePtr& piece)
        {
            {
                SpinlocksMutexLocker locker(ops_mutex_);
                AddOperation(piece_index,
                        std::tr1::bind(
                            &FileService::AddReadOperation,
                            this, piece, piece_index, _1, _2, _3, _4));
            }
            ops_event_.SetEvent();
        }

        void WritePiece(std::size_t piece_index, const PiecePtr& piece)
        {
            {
                SpinlocksMutexLocker locker(ops_mutex_);
                AddOperation(piece_index,
                        std::tr1::bind(
                            &FileService::AddWriteOperation,
                            this, piece, piece_index, _1, _2, _3, _4));
            }
            ops_event_.SetEvent();
        }

        void FlushFileBuffer()
        {
            {
                SpinlocksMutexLocker locker(ops_mutex_);
                AddFlushOperation();
            }
            ops_event_.SetEvent();
        }

        void GetReadPieces(std::map<std::size_t, PiecePtr>& read_pieces)
        {
            SpinlocksMutexLocker locker(res_mutex_);
            read_pieces.swap(read_res_);
        }

        void GetWritedPieces(std::vector<std::size_t>& writed_pieces)
        {
            SpinlocksMutexLocker locker(res_mutex_);
            writed_pieces.swap(write_res_);
        }

    private:
        typedef std::tr1::shared_ptr<File> FilePtr;

        class Operation
        {
        public:
            enum Op_Type
            {
                READ,   // read data
                WRITE,  // write data
                FLUSH   // flush file buffer
            };

            // just for Flush
            explicit Operation(Op_Type ot)
                : op_type(ot)
            {
            }

            Operation(Op_Type ot,
                      const PiecePtr& p,
                      std::size_t pi,
                      std::size_t pip,
                      std::size_t fi,
                      long long pif,
                      long long bytes)
                : op_type(ot),
                  piece(p),
                  piece_index(pi),
                  pos_in_piece(pip),
                  file_index(fi),
                  pos_in_file(pif),
                  op_bytes(bytes)
            {
            }

            void CompleteOperation(std::vector<FilePtr>& files)
            {
                if (op_type == READ)
                {
                    files[file_index]->Read(pos_in_file, op_bytes,
                                            piece->GetRawDataPtr() + pos_in_piece);
                }
                else if (op_type == WRITE)
                {
                    files[file_index]->Write(pos_in_file, op_bytes,
                                             piece->GetRawDataPtr() + pos_in_piece);
                }
                else
                {
                    // flush all files
                    std::for_each(files.begin(), files.end(),
                            std::tr1::bind(&File::Flush,
                                std::tr1::placeholders::_1));
                }
            }

            void CollectResult(std::map<std::size_t, PiecePtr>& read_result,
                               std::vector<std::size_t>& write_result)
            {
                if (op_type == READ)
                    read_result.insert(std::make_pair(piece_index, piece));
                else if (op_type == WRITE)
                    write_result.push_back(piece_index);
            }

        private:
            Op_Type op_type;
            PiecePtr piece;
            std::size_t piece_index;
            std::size_t pos_in_piece;
            std::size_t file_index;
            long long pos_in_file;
            long long op_bytes;
        };

        void PrepareFiles(const std::tr1::shared_ptr<BitData>& bitdata)
        {
            std::string base_path = bitdata->GetBasePath();
            const BitData::DownloadFiles& files_info = bitdata->GetFilesInfo();

            long long boundary = 0ll;
            BitData::DownloadFiles::const_iterator it = files_info.begin();
            for (; it != files_info.end(); ++it)
            {
                FilePtr file(new File(
                            base_path + it->file_path,
                            it->length, it->is_download));
                file_group_.push_back(file);

                boundary += it->length;
                file_boundary_.push_back(boundary);
                file_size_.push_back(it->length);
            }
        }

        void PrepareIoThread()
        {
            io_thread_.Reset(
                    new Thread(std::tr1::bind(
                            &FileService::IoThreadFunc, this)));
        }

        unsigned IoThreadFunc()
        {
            while (true)
            {
                if (ops_event_.WaitForever())
                {
                    if (AtomicAdd(&thread_exit_flag_, 0))
                        break;

                    std::vector<Operation> operations;
                    {
                        SpinlocksMutexLocker locker(ops_mutex_);
                        operations.swap(operations_);
                    }

                    std::for_each(operations.begin(), operations.end(),
                            std::tr1::bind(&Operation::CompleteOperation,
                                _1, std::tr1::ref(file_group_)));

                    {
                        SpinlocksMutexLocker locker(res_mutex_);
                        std::for_each(operations.begin(), operations.end(),
                                std::tr1::bind(&Operation::CollectResult,
                                    _1, std::tr1::ref(read_res_), std::tr1::ref(write_res_)));
                    }
                }
            }

            return 0;
        }

        std::pair<long long, long long> GetPieceByteRange(long long piece_index) const
        {
            return std::make_pair(piece_index * piece_length_,
                                  (piece_index + 1) * piece_length_);
        }

        std::pair<std::size_t, std::size_t> GetFileIndexRange(long long piece_index) const
        {
            std::pair<long long, long long> byte_range = GetPieceByteRange(piece_index);
            // let second as the last byte index
            --byte_range.second;

            std::vector<long long>::const_iterator first_it = std::upper_bound(
                    file_boundary_.begin(), file_boundary_.end(), byte_range.first);
            std::vector<long long>::const_iterator last_it = std::upper_bound(
                    file_boundary_.begin(), file_boundary_.end(), byte_range.second);

            std::size_t first_file_index = std::distance(
                    file_boundary_.begin(), first_it);
            std::size_t last_file_index = std::distance(
                    file_boundary_.begin(), last_it);

            return std::make_pair(first_file_index, last_file_index);
        }

        template<typename OperationAdder>
        void AddOperation(std::size_t piece_index, const OperationAdder& op_adder)
        {
            std::pair<long long, long long> byte_range =
                GetPieceByteRange(piece_index);

            std::pair<std::size_t, std::size_t> file_index =
                GetFileIndexRange(piece_index);

            long long pos_in_file = 0;
            for (std::size_t i = 0; i < file_index.first; ++i)
                pos_in_file += file_size_[i];
            pos_in_file = byte_range.first - pos_in_file;

            long long pos_in_piece = 0;
            long long left_bytes = byte_range.second - byte_range.first;
            for (; file_index.first <= file_index.second; ++file_index.first)
            {
                long long file_left_bytes = file_size_[file_index.first] - pos_in_file;
                long long op_bytes = left_bytes > file_left_bytes ?
                    file_left_bytes : left_bytes;

                op_adder(pos_in_piece, file_index.first, pos_in_file, op_bytes);

                pos_in_piece += op_bytes;
                left_bytes -= op_bytes;
                pos_in_file = 0;
            }
        }

        void AddReadOperation(const PiecePtr& piece,
                              std::size_t piece_index,
                              long long pos_in_piece,
                              std::size_t file_index,
                              long long pos_in_file,
                              long long read_bytes)
        {
            operations_.push_back(
                    Operation(Operation::READ,
                              piece,
                              piece_index,
                              static_cast<std::size_t>(pos_in_piece),
                              file_index,
                              pos_in_file,
                              read_bytes));
        }

        void AddWriteOperation(const PiecePtr& piece,
                               std::size_t piece_index,
                               long long pos_in_piece,
                               std::size_t file_index,
                               long long pos_in_file,
                               long long write_bytes)
        {
            operations_.push_back(
                    Operation(Operation::WRITE,
                              piece,
                              piece_index,
                              static_cast<std::size_t>(pos_in_piece),
                              file_index,
                              pos_in_file,
                              write_bytes));
        }

        void AddFlushOperation()
        {
            operations_.push_back(Operation(Operation::FLUSH));
        }

        long long piece_length_;
        std::vector<long long> file_boundary_;
        std::vector<long long> file_size_;

        volatile long thread_exit_flag_;
        std::vector<FilePtr> file_group_;
        ScopePtr<Thread> io_thread_;
        AutoResetEvent ops_event_;
        SpinlocksMutex ops_mutex_;
        SpinlocksMutex res_mutex_;
        std::map<std::size_t, PiecePtr> read_res_;
        std::vector<std::size_t> write_res_;
        std::vector<Operation> operations_;
    };

    BitFile::BitFile(const std::tr1::shared_ptr<BitData>& bitdata)
        : bitdata_(bitdata),
          file_service_(new FileService(bitdata))
    {
    }

    void BitFile::ReadPiece(std::size_t piece_index, const PiecePtr& piece)
    {
        file_service_->ReadPiece(piece_index, piece);
    }

    void BitFile::GetReadPieces(std::map<std::size_t, PiecePtr>& read_pieces)
    {
        file_service_->GetReadPieces(read_pieces);
    }

    void BitFile::WritePiece(std::size_t piece_index, const PiecePtr& piece)
    {
        file_service_->WritePiece(piece_index, piece);
    }

    void BitFile::GetWritedPieces(std::vector<std::size_t>& writed_pieces)
    {
        file_service_->GetWritedPieces(writed_pieces);
    }

    void BitFile::FlushFileBuffer()
    {
        file_service_->FlushFileBuffer();
    }

} // namespace core
} // namespace bitwave

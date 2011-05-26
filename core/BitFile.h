#ifndef BIT_FILE_H
#define BIT_FILE_H

#include "../base/BaseTypes.h"
#include <memory>
#include <map>
#include <vector>

namespace bitwave {
namespace core {

    class BitData;
    class BitPiece;

    class BitFile : private NotCopyable
    {
    public:
        class CreateFileError
        {
        public:
            enum Error
            {
                PATH_ERROR,
                SPACE_NOT_ENOUGH,
            };

            explicit CreateFileError(Error error)
                : error_(error)
            {
            }

            Error GetErrorCode() const
            {
                return error_;
            }

        private:
            Error error_;
        };

        typedef std::tr1::shared_ptr<BitPiece> PiecePtr;

        explicit BitFile(const std::tr1::shared_ptr<BitData>& bitdata);

        void ReadPiece(std::size_t piece_index, const PiecePtr& piece);

        void GetReadPieces(std::map<std::size_t, PiecePtr>& read_pieces);

        void WritePiece(std::size_t piece_index, const PiecePtr& piece);

        void GetWritedPieces(std::vector<std::size_t>& writed_pieces);

        void FlushFileBuffer();

    private:
        class FileService;

        std::tr1::shared_ptr<BitData> bitdata_;
        std::tr1::shared_ptr<FileService> file_service_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_FILE_H

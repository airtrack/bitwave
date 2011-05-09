#include "BitPieceSha1Calc.h"
#include "BitPiece.h"
#include "../thread/Atomic.h"

namespace bittorrent {
namespace core {

    BitPieceSha1Calc::BitPieceSha1Calc()
        : thread_exit_flag_(0)
    {
        sha1_thread_.Reset(
                new Thread(std::tr1::bind(
                    &BitPieceSha1Calc::CalcSha1Thread, this)));
    }

    BitPieceSha1Calc::~BitPieceSha1Calc()
    {
        AtomicAdd(&thread_exit_flag_, 1);
        piece_list_event_.SetEvent();
        sha1_thread_->Join();
    }

    void BitPieceSha1Calc::GetResult(PieceSha1List& sha1_list)
    {
        SpinlocksMutexLocker locker(piece_sha1_list_mutex_);
        sha1_list.swap(piece_sha1_list_);
    }

    void BitPieceSha1Calc::AddPiece(std::size_t piece_index,
                                    const std::tr1::shared_ptr<BitPiece>& piece)
    {
        SpinlocksMutexLocker locker(piece_list_mutex_);
        piece_list_.push_back(std::make_pair(piece_index, piece));
        piece_list_event_.SetEvent();
    }

    unsigned BitPieceSha1Calc::CalcSha1Thread()
    {
        while (true)
        {
            if (piece_list_event_.WaitForever())
            {
                if (AtomicAdd(&thread_exit_flag_, 0))
                    break;

                PieceList piece_list;
                {
                    SpinlocksMutexLocker locker(piece_list_mutex_);
                    piece_list.swap(piece_list_);
                }

                PieceSha1List piece_sha1_list;
                CalculateSha1(piece_list, piece_sha1_list);
                {
                    SpinlocksMutexLocker locker(piece_sha1_list_mutex_);
                    piece_sha1_list_.insert(piece_sha1_list_.end(),
                            piece_sha1_list.begin(), piece_sha1_list.end());
                }
            }
        }

        return 0;
    }

    void BitPieceSha1Calc::CalculateSha1(PieceList& piece_list,
                                         PieceSha1List& piece_sha1_list)
    {
        for (PieceList::iterator it = piece_list.begin();
                it != piece_list.end(); ++it)
        {
            const char *data = it->second->GetRawDataPtr();
            std::size_t size = it->second->GetSize();
            piece_sha1_list.push_back(
                    std::make_pair(it->first,
                        Sha1Value(data, size)));
        }
    }

} // namespace core
} // namespace bittorrent

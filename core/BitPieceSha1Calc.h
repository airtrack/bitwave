#ifndef BIT_PIECE_SHA1_CALC_H
#define BIT_PIECE_SHA1_CALC_H

#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../sha1/Sha1Value.h"
#include "../thread/Thread.h"
#include "../thread/Event.h"
#include "../thread/Mutex.h"
#include <functional>
#include <memory>
#include <vector>
#include <utility>

namespace bitwave {
namespace core {

    class BitPiece;

    class BitPieceSha1Calc : private NotCopyable
    {
    public:
        typedef std::vector<
            std::pair<std::size_t, std::tr1::shared_ptr<BitPiece>>> PieceList;
        typedef std::vector<
            std::pair<std::size_t, Sha1Value>> PieceSha1List;

        BitPieceSha1Calc();
        ~BitPieceSha1Calc();

        void GetResult(PieceSha1List& sha1_list);

        void AddPiece(std::size_t piece_index,
                      const std::tr1::shared_ptr<BitPiece>& piece);

    private:
        unsigned CalcSha1Thread();
        void CalculateSha1(PieceList& piece_list,
                           PieceSha1List& piece_sha1_list);

        volatile long thread_exit_flag_;
        ScopePtr<Thread> sha1_thread_;
        AutoResetEvent piece_list_event_;
        SpinlocksMutex piece_list_mutex_;
        SpinlocksMutex piece_sha1_list_mutex_;
        PieceList piece_list_;
        PieceSha1List piece_sha1_list_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_PIECE_SHA1_CALC_H

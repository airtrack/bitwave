#ifndef BIT_PIECE_H
#define BIT_PIECE_H

#include "../base/BaseTypes.h"
#include <vector>
#include <utility>

namespace bittorrent {
namespace core {

    class BitPiece : private NotCopyable
    {
    public:
        enum State
        {
            NOT_CHECKED,
            CHECKING_SHA1,
            CHECK_SHA1_OK,
            WRITED,
        };

        explicit BitPiece(std::size_t size)
            : data_(size),
              state_(NOT_CHECKED)
        {
        }

        const char * GetRawDataPtr() const
        {
            return &data_[0];
        }

        char * GetRawDataPtr()
        {
            return &data_[0];
        }

        std::size_t GetSize() const
        {
            return data_.size();
        }

        void SetState(State state)
        {
            state_ = state;
        }

        State GetState() const
        {
            return state_;
        }

        void WriteBlock(std::size_t begin,
                        std::size_t length,
                        const char *block);

        bool IsComplete() const;

    private:
        void MarkWriteBlock(std::size_t begin,
                            std::size_t end);
        void MergeNextWriteBlock(std::size_t cur);

        std::vector<char> data_;
        std::vector<std::pair<std::size_t, std::size_t>> writed_;
        State state_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_PIECE_H

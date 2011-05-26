#ifndef BIT_PIECE_H
#define BIT_PIECE_H

#include "../base/BaseTypes.h"
#include "../timer/TimeTraits.h"
#include <string.h>
#include <vector>
#include <utility>

namespace bitwave {
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
              state_(NOT_CHECKED),
              last_read_time_(time_traits<NormalTimeType>::invalid()),
              read_times_(0)
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

        void TouchRead()
        {
            ++read_times_;
            last_read_time_ = time_traits<NormalTimeType>::now();
        }

        void Clear()
        {
            memset(&data_[0], 0, data_.size());
            writed_.clear();
            state_ = NOT_CHECKED;
            read_times_ = 0;
            last_read_time_ = time_traits<NormalTimeType>::invalid();
        }

        void WriteBlock(std::size_t begin,
                        std::size_t length,
                        const char *block);

        bool IsComplete() const;

        static bool IsReadTimeOld(const BitPiece& left,
                                  const BitPiece& right)
        {
            return time_traits<NormalTimeType>::less(
                    left.last_read_time_, right.last_read_time_);
        }

        static bool IsReadTimeEqual(const BitPiece& left,
                                    const BitPiece& right)
        {
            return time_traits<NormalTimeType>::equal(
                    left.last_read_time_, right.last_read_time_);
        }

        static bool IsReadTimesLess(const BitPiece& left,
                                    const BitPiece& right)
        {
            return left.read_times_ < right.read_times_;
        }

    private:
        void MarkWriteBlock(std::size_t begin,
                            std::size_t end);
        void MergeNextWriteBlock(std::size_t cur);

        std::vector<char> data_;
        std::vector<std::pair<std::size_t, std::size_t>> writed_;
        State state_;
        NormalTimeType last_read_time_;
        std::size_t read_times_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_PIECE_H

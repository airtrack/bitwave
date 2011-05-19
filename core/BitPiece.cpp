#include "BitPiece.h"
#include <string.h>
#include <iterator>

namespace bittorrent {
namespace core {

    void BitPiece::WriteBlock(std::size_t begin,
                              std::size_t length,
                              const char *block)
    {
        if (state_ != NOT_CHECKED)
            return ;

        if (begin >= data_.size() || begin + length > data_.size())
            return ;

        memcpy(&data_[begin], block, length);
        MarkWriteBlock(begin, begin + length);
    }

    bool BitPiece::IsComplete() const
    {
        if (writed_.empty())
            return false;

        if (writed_[0].first == 0 && writed_[0].second == data_.size())
            return true;

        return false;
    }

    void BitPiece::MarkWriteBlock(std::size_t begin,
                                  std::size_t end)
    {
        std::size_t i = 0;
        for (; i < writed_.size(); ++i)
        {
            if (writed_[i].first <= begin && writed_[i].second >= end)
            {
                // do nothing
                return ;
            }
            else if (writed_[i].first == end)
            {
                writed_[i].first = begin;
                return ;
            }
            else if (writed_[i].second == begin)
            {
                writed_[i].second = end;
                MergeNextWriteBlock(i);
                return ;
            }
            else
            {
                if (end < writed_[i].first)
                    break;
            }
        }

        std::vector<std::pair<std::size_t, std::size_t>>::iterator it =
            writed_.begin();
        std::advance(it, i);
        writed_.insert(it, std::make_pair(begin, end));
    }

    void BitPiece::MergeNextWriteBlock(std::size_t cur)
    {
        std::size_t next = cur + 1;
        if (next >= writed_.size())
            return ;

        if (writed_[cur].second == writed_[next].first)
        {
            writed_[cur].second = writed_[next].second;
            std::vector<std::pair<std::size_t, std::size_t>>::iterator it =
                writed_.begin();
            std::advance(it, next);
            writed_.erase(it);
        }
    }

} // namespace core
} // namespace bittorrent

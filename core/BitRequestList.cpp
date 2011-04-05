#include "BitRequestList.h"
#include <algorithm>

namespace bittorrent {
namespace core {

    BitRequestList::BitRequestList(std::size_t max_request)
        : max_request_(max_request)
    {
    }

    bool BitRequestList::AddRequest(int index, int begin, int length)
    {
        if (request_list_.size() >= max_request_)
            return false;

        request_list_.push_back(RequestData(index, begin, length));
        return true;
    }

    void BitRequestList::DelRequest(int index, int begin, int length)
    {
        request_list_.remove(RequestData(index, begin, length));
    }

    BitRequestList::Iterator BitRequestList::FindRequest(int index, int begin, int length)
    {
        return std::find(Begin(), End(), RequestData(index, begin, length));
    }

    BitRequestList::Iterator BitRequestList::Splice(BitRequestList& brl, Iterator it)
    {
        request_list_.splice(End(), brl.request_list_, it);
        return --End();
    }

} // namespace core
} // namespace bittorrent

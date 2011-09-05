#include "BitRequestList.h"
#include <algorithm>

namespace bitwave {
namespace core {

    void BitRequestList::AddRequest(int index, int begin, int length)
    {
        request_list_.push_back(RequestData(index, begin, length));
    }

    void BitRequestList::AddRequest(Iterator first, Iterator last)
    {
        for (; first != last; ++first)
            AddRequest(first->index, first->begin, first->length);
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

    void BitRequestList::Splice(BitRequestList& brl, Iterator first, Iterator last)
    {
        request_list_.splice(End(), brl.request_list_, first, last);
    }

    bool BitRequestList::IsExistRequest(int index, int begin, int length) const
    {
        return std::find(Begin(), End(), RequestData(index, begin, length)) != End();
    }

} // namespace core
} // namespace bitwave

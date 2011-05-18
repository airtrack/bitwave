#ifndef BIT_REQUEST_LIST_H
#define BIT_REQUEST_LIST_H

#include "../base/BaseTypes.h"
#include <list>
#include <limits>

namespace bittorrent {
namespace core {

    class BitRequestList : private NotCopyable
    {
    public:
        struct RequestData
        {
            RequestData(int i, int b, int l)
                : index(i),
                  begin(b),
                  length(l)
            {
            }

            int index;      // zero-base piece index
            int begin;      // zero-base byte offset within the piece
            int length;     // request length

            friend bool operator == (const RequestData& left,
                                     const RequestData& right)
            {
                return left.index == right.index &&
                       left.begin == right.begin &&
                       left.length == right.length;
            }
        };

        typedef std::list<RequestData> RequestList;
        typedef RequestList::iterator Iterator;
        typedef RequestList::const_iterator Const_Iterator;

        // add new request to tail
        void AddRequest(int index, int begin, int length);

        // delete request if exists
        void DelRequest(int index, int begin, int length);

        // find request data, if exists return the Iterator, otherwise return End()
        Iterator FindRequest(int index, int begin, int length);

        // splice the element of it from argument brl BitRequestList to
        // target BitRequestList's tail, return the element Iterator in
        // target BitRequestList
        Iterator Splice(BitRequestList& brl, Iterator it);

        // return the request is exist or not
        bool IsExistRequest(int index, int begin, int length) const;

        void Clear() { request_list_.clear(); }
        std::size_t Size() const { return request_list_.size(); }
        bool Empty() const { return request_list_.empty(); }
        void Erase(Iterator it) { request_list_.erase(it); }
        Iterator Begin() { return request_list_.begin(); }
        Iterator End() { return request_list_.end(); }
        Const_Iterator Begin() const { return request_list_.begin(); }
        Const_Iterator End() const { return request_list_.end(); }

    private:
        RequestList request_list_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_REQUEST_LIST_H

#ifndef BIT_DOWNLOAD_DISPATCHER_H
#define BIT_DOWNLOAD_DISPATCHER_H

#include "BitRequestList.h"
#include "../base/BaseTypes.h"
#include <memory>

namespace bittorrent {
namespace core {

    class BitData;
    class BitPeerData;

    class BitDownloadDispatcher : private NotCopyable
    {
    public:
        explicit BitDownloadDispatcher(
                const std::tr1::shared_ptr<BitData>& bitdata);

        void DispatchRequestList(
                const std::tr1::shared_ptr<BitPeerData>& peer_data,
                BitRequestList& request_list);

        void ReturnRequest(BitRequestList& request_list,
                           BitRequestList::Iterator it);

    private:
        // the BitData of associate BitTask
        std::tr1::shared_ptr<BitData> bitdata_;
        // scattered block requests in some piece
        BitRequestList scattered_request_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_DOWNLOAD_DISPATCHER_H

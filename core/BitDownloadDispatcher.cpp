#include "BitDownloadDispatcher.h"
#include "BitData.h"
#include "BitPeerData.h"

namespace bittorrent {
namespace core {

    BitDownloadDispatcher::BitDownloadDispatcher(
            const std::tr1::shared_ptr<BitData>& bitdata)
        : bitdata_(bitdata)
    {
    }

    void BitDownloadDispatcher::DispatchRequestList(
            const std::tr1::shared_ptr<BitPeerData>& peer_data,
            BitRequestList& request_list)
    {
    }

    void BitDownloadDispatcher::ReturnRequest(BitRequestList& request_list,
                                              BitRequestList::Iterator it)
    {
    }

} // namespace core
} // namespace bittorrent

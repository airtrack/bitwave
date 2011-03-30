#ifndef BIT_PEER_DATA_H
#define BIT_PEER_DATA_H

#include "../base/BaseTypes.h"
#include <string>

namespace bittorrent {
namespace core {

    class BitPeerData : private NotCopyable
    {
    public:
        explicit BitPeerData(const std::string& peer_id);

    private:
        std::string peer_id_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_PEER_DATA_H

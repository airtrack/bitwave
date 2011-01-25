#include "BitPeerData.h"

namespace bittorrent {
namespace core {

    BitPeerData::BitPeerData(unsigned long ip, unsigned short port)
        : ip_(ip), port_(port)
    {
    }

    unsigned long BitPeerData::GetIp() const
    {
        return ip_;
    }

    unsigned short BitPeerData::GetPort() const
    {
        return port_;
    }

} // namespace core
} // namespace bittorrent

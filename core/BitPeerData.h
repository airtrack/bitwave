#ifndef BIT_PEER_DATA_H
#define BIT_PEER_DATA_H

#include "../base/BaseTypes.h"

namespace bittorrent {
namespace core {

    class BitPeerData : private NotCopyable
    {
    public:
        BitPeerData(unsigned long ip, unsigned short port);

        unsigned long GetIp() const;
        unsigned short GetPort() const;

    private:
        const unsigned long ip_;
        const unsigned short port_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_PEER_DATA_H

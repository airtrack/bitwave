#ifndef BIT_PEER_CONNECTION_H
#define BIT_PEER_CONNECTION_H

#include "../base/BaseTypes.h"
#include "../net/IoService.h"
#include "../net/StreamUnpacker.h"
#include <memory>

namespace bittorrent {
namespace core {

    // peer wire protocol unpack ruler
    class PeerProtocolUnpackRuler
    {
    public:
        static bool CanUnpack(const char *stream,
            std::size_t size, std::size_t *pack_len);
    };

    class BitPeerData;
    class BitPeerConnection;

    class PeerConnectionOwner
    {
    public:
        virtual void LetMeLeave(const std::tr1::shared_ptr<BitPeerConnection>& child) = 0;
    };

    class BitPeerConnection : public std::tr1::enable_shared_from_this<BitPeerConnection>,
                              private NotCopyable
    {
    public:
        typedef std::tr1::shared_ptr<BitPeerData> PeerDataPtr;

        BitPeerConnection(const net::SocketHandler& socket,
                          PeerConnectionOwner *owner);

    private:
        struct ConnectionState
        {
            ConnectionState()
                : am_choking(true),
                  am_interested(false),
                  peer_choking(true),
                  peer_interested(false)
            {}

            bool am_choking;
            bool am_interested;
            bool peer_choking;
            bool peer_interested;
        };

        ConnectionState connection_state_;
        net::SocketHandler socket_;
        net::IoService& io_service_;
        PeerDataPtr peer_data_;
        bool connecting_;
        PeerConnectionOwner *owner_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_PEER_CONNECTION_H

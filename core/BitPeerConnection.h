#ifndef BIT_PEER_CONNECTION_H
#define BIT_PEER_CONNECTION_H

#include "BitNetProcessor.h"
#include "../base/BaseTypes.h"
#include <memory>

namespace bittorrent {
namespace core {

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
        BitPeerConnection(const net::AsyncSocket& socket,
                          PeerConnectionOwner *owner);

        BitPeerConnection(const net::Address& remote_address,
                          const net::Port& remote_listen_port,
                          net::IoService& io_service,
                          PeerConnectionOwner *owner);

    private:
        // peer wire protocol unpack ruler
        class PeerProtocolUnpackRuler
        {
        public:
            static bool CanUnpack(const char *stream,
                    std::size_t size, std::size_t *pack_len);
        };

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

        void BindNetProcessorCallbacks();
        void ConnectClosed();
        void ProcessProtocol(const char *data, std::size_t size);

        PeerConnectionOwner *owner_;
        ConnectionState connection_state_;
        std::tr1::shared_ptr<BitPeerData> peer_data_;
        BitNetProcessor<PeerProtocolUnpackRuler> net_processor_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_PEER_CONNECTION_H

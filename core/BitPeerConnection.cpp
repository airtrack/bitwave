#include "BitPeerConnection.h"
#include "BitPeerData.h"
#include "../net/NetHelper.h"
#include <assert.h>
#include <string.h>
#include <functional>

namespace bittorrent {
namespace core {

    // protocol string
    const char protocol_string[] = "BitTorrent protocol";
    const std::size_t protocol_string_len = sizeof(protocol_string) - 1;
    // handshake protocol size
    const std::size_t handshake_size = 49 + protocol_string_len;

    // static
    bool BitPeerConnection::PeerProtocolUnpackRuler::CanUnpack(const char *stream,
        std::size_t size, std::size_t *pack_len)
    {
        assert(stream && size > 0);
        assert(pack_len);
        
        if (size < 4)
            return false;

        // is handshake protocol
        if (*stream == protocol_string_len && size >= handshake_size)
        {
            if (memcmp(stream + 1, protocol_string, protocol_string_len) == 0)
            {
                *pack_len = handshake_size;
                return true;
            }
        }

        unsigned long length_prefix = net::NetToHostl(
            *reinterpret_cast<const unsigned long *>(stream));
        if (size >= length_prefix + 4)
        {
            *pack_len = length_prefix + 4;
            return true;
        }

        return false;
    }

    BitPeerConnection::BitPeerConnection(const net::AsyncSocket& socket,
                                         PeerConnectionOwner *owner)
        : owner_(owner),
          net_processor_(new NetProcessor(socket))
    {
        assert(owner_);
        BindNetProcessorCallbacks();
        net_processor_->Receive();
    }

    BitPeerConnection::BitPeerConnection(const net::Address& remote_address,
                                         const net::Port& remote_listen_port,
                                         net::IoService& io_service,
                                         PeerConnectionOwner *owner)
        : owner_(owner),
          net_processor_(new NetProcessor(io_service))
    {
        assert(owner_);
        BindNetProcessorCallbacks();
        net_processor_->Connect(remote_address, remote_listen_port);
    }

    void BitPeerConnection::BindNetProcessorCallbacks()
    {
        net_processor_->SetConnectCallback(
                std::tr1::bind(&BitPeerConnection::Connected, this));
        net_processor_->SetDisconnectCallback(
                std::tr1::bind(&BitPeerConnection::ConnectClosed, this));
        net_processor_->SetProtocolCallback(
                std::tr1::bind(&BitPeerConnection::ProcessProtocol, this,
                    std::tr1::placeholders::_1, std::tr1::placeholders::_2));
    }

    void BitPeerConnection::Connected()
    {
    }

    void BitPeerConnection::ConnectClosed()
    {
    }

    void BitPeerConnection::ProcessProtocol(const char *data, std::size_t size)
    {
    }

} // namespace core
} // namespace bittorrent

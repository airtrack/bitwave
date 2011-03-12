#include "BitPeerConnection.h"
#include "BitPeerData.h"
#include "../net/NetHelper.h"
#include <assert.h>
#include <string.h>

namespace bittorrent {
namespace core {

    // protocol string
    const char protocol_string[] = "BitTorrent protocol";
    const std::size_t protocol_string_len = sizeof(protocol_string) - 1;
    // handshake protocol size
    const std::size_t handshake_size = 49 + protocol_string_len;

    // static
    bool PeerProtocolUnpackRuler::CanUnpack(const char *stream,
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

    BitPeerConnection::BitPeerConnection(const net::SocketHandler& socket,
                                         PeerConnectionOwner *owner)
        : socket_(socket),
          io_service_(socket.GetIoService()),
          connecting_(true),
          owner_(owner)
    {
        assert(owner_);
    }

} // namespace core
} // namespace bittorrent

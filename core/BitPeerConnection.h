#ifndef BIT_PEER_CONNECTION_H
#define BIT_PEER_CONNECTION_H

#include "BitNetProcessor.h"
#include "../base/BaseTypes.h"
#include "../sha1/Sha1Value.h"
#include <memory>
#include <string>

namespace bittorrent {
namespace core {

    class BitData;
    class BitPeerData;
    class BitPeerConnection;

    // BitPeerConnection's parent interface
    class PeerConnectionOwner
    {
    public:
        virtual void LetMeLeave(const std::tr1::shared_ptr<BitPeerConnection>& child) = 0;
    };

    // manage peer connection and all operations
    class BitPeerConnection : public std::tr1::enable_shared_from_this<BitPeerConnection>,
                              private NotCopyable
    {
    public:
        BitPeerConnection(const net::AsyncSocket& socket,
                          PeerConnectionOwner *owner);

        BitPeerConnection(const std::shared_ptr<BitData>& bitdata,
                          const net::Address& remote_address,
                          const net::Port& remote_listen_port,
                          net::IoService& io_service,
                          PeerConnectionOwner *owner);

        ~BitPeerConnection();

        void SetOwner(PeerConnectionOwner *owner);

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
        void ClearNetProcessor();
        void Connected();
        void ConnectClosed();
        void ProcessProtocol(const char *data, std::size_t size);
        bool ProcessHandshake(const char *data);
        void ProcessMessage(const char *data, std::size_t len);
        void ProcessKeepAlive();
        void ProcessChoke(bool choke);
        void ProcessInterested(bool interest);
        void ProcessHave(const char *data, std::size_t len);
        void ProcessBitfield(const char *data, std::size_t len);
        void ProcessRequest(const char *data, std::size_t len);
        void ProcessPiece(const char *data, std::size_t len);
        void ProcessCancel(const char *data, std::size_t len);

        bool AttachTask(const Sha1Value& info_hash);
        void PreparePeerData(const std::string& peer_id);
        void DropConnection();
        void SendHandshake();
        void MarkPeerHavePiece(int piece_index);

        typedef BitNetProcessor<PeerProtocolUnpackRuler> NetProcessor;

        PeerConnectionOwner *owner_;
        ConnectionState connection_state_;
        std::tr1::shared_ptr<BitData> bitdata_;
        std::tr1::shared_ptr<BitPeerData> peer_data_;
        std::tr1::shared_ptr<NetProcessor> net_processor_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_PEER_CONNECTION_H

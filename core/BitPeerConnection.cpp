#include "BitPeerConnection.h"
#include "BitTask.h"
#include "BitData.h"
#include "BitPeerData.h"
#include "BitService.h"
#include "BitController.h"
#include "../net/NetHelper.h"
#include "../sha1/NetSha1Value.h"
#include <assert.h>
#include <string.h>
#include <functional>

namespace {

    // protocol string
    const char protocol_string[] = "BitTorrent protocol";
    const std::size_t protocol_string_len = sizeof(protocol_string) - 1;
    const std::size_t protocol_reserved = 8;
    // handshake protocol size
    const std::size_t handshake_size = 49 + protocol_string_len;

    enum PEER_MESSAGE
    {
        KEEP_ALIVE = -1,
        CHOKE,
        UNCHOKE,
        INTERESTED,
        NOT_INTERESTED,
        HAVE,
        BITFIELD,
        REQUEST,
        PIECE,
        CANCEL,
        PORT
    };

} // unnamed namespace

namespace bittorrent {
namespace core {

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

    BitPeerConnection::BitPeerConnection(const std::shared_ptr<BitData>& bitdata,
                                         const net::Address& remote_address,
                                         const net::Port& remote_listen_port,
                                         net::IoService& io_service,
                                         PeerConnectionOwner *owner)
        : owner_(owner),
          bitdata_(bitdata),
          net_processor_(new NetProcessor(io_service))
    {
        assert(owner_);
        BindNetProcessorCallbacks();
        net_processor_->Connect(remote_address, remote_listen_port);
    }

    BitPeerConnection::~BitPeerConnection()
    {
        ClearNetProcessor();
        if (bitdata_) bitdata_->DelPeerData(peer_data_);
    }

    void BitPeerConnection::SetOwner(PeerConnectionOwner *owner)
    {
        assert(owner);
        owner_ = owner;
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

    void BitPeerConnection::ClearNetProcessor()
    {
        if (net_processor_)
        {
            net_processor_->ClearProtocolCallback();
            net_processor_->ClearConnectCallback();
            net_processor_->ClearDisconnectCallback();
            net_processor_->Close();
            net_processor_.reset();
        }
    }

    void BitPeerConnection::Connected()
    {
        SendHandshake();
    }

    void BitPeerConnection::ConnectClosed()
    {
        DropConnection();
    }

    void BitPeerConnection::ProcessProtocol(const char *data, std::size_t size)
    {
        assert(data);
        if (size == handshake_size && ProcessHandshake(data))
            return ;

        assert(size >= sizeof(long));
        unsigned long length_prefix = net::NetToHostl(
                *reinterpret_cast<const unsigned long *>(data));
        ProcessMessage(data + sizeof(long), length_prefix);
    }

    bool BitPeerConnection::ProcessHandshake(const char *data)
    {
        if (*data != protocol_string_len ||
            memcmp(data + 1, protocol_string, protocol_string_len) != 0)
            return false;

        const char *info_hash_ptr = data + 1 + protocol_string_len + protocol_reserved;
        const char *peer_id_ptr = info_hash_ptr + 20;

        Sha1Value info_hash = NetStreamToSha1Value(info_hash_ptr);
        std::string peer_id(peer_id_ptr, 20);

        if (bitdata_)
        {
            if (bitdata_->GetInfoHash() != info_hash)
            {
                DropConnection();
                return false;
            }
        }
        else
        {
            if (!AttachTask(info_hash))
            {
                DropConnection();
                return false;
            }

            // send back handshake
            SendHandshake();
        }

        PreparePeerData(peer_id);
        return true;
    }

    void BitPeerConnection::ProcessMessage(const char *data, std::size_t len)
    {
        int message = len == 0 ? KEEP_ALIVE : *data++;
        switch (message)
        {
        case KEEP_ALIVE:     ProcessKeepAlive();             break;
        case CHOKE:          ProcessChoke(true);             break;
        case UNCHOKE:        ProcessChoke(false);            break;
        case INTERESTED:     ProcessInterested(true);        break;
        case NOT_INTERESTED: ProcessInterested(false);       break;
        case HAVE:           ProcessHave(data, len - 1);     break;
        case BITFIELD:       ProcessBitfield(data, len - 1); break;
        case REQUEST:        ProcessRequest(data, len - 1);  break;
        case PIECE:          ProcessPiece(data, len - 1);    break;
        case CANCEL:         ProcessCancel(data, len - 1);   break;
        case PORT:
        default:
            // PORT message and other messages are not supported, do nothing
            break;
        }
    }

    void BitPeerConnection::ProcessKeepAlive()
    {
    }

    void BitPeerConnection::ProcessChoke(bool choke)
    {
        connection_state_.peer_choking = choke;
    }

    void BitPeerConnection::ProcessInterested(bool interest)
    {
        connection_state_.peer_interested = interest;
    }

    void BitPeerConnection::ProcessHave(const char *data, std::size_t len)
    {
        if (len != sizeof(int))
            return ;
        int piece_index = net::NetToHosti(*reinterpret_cast<const int *>(data));
        MarkPeerHavePiece(piece_index);
    }

    void BitPeerConnection::ProcessBitfield(const char *data, std::size_t len)
    {
        int piece_index = 0;
        for (std::size_t i = 0; i < len; ++i)
        {
            for (int j = 7; j >= 0; --j, ++piece_index)
                if ((data[i] >> j) & 0x01)
                    MarkPeerHavePiece(piece_index);
        }
    }

    void BitPeerConnection::ProcessRequest(const char *data, std::size_t len)
    {
    }

    void BitPeerConnection::ProcessPiece(const char *data, std::size_t len)
    {
    }

    void BitPeerConnection::ProcessCancel(const char *data, std::size_t len)
    {
    }

    bool BitPeerConnection::AttachTask(const Sha1Value& info_hash)
    {
        assert(BitService::controller);
        std::tr1::shared_ptr<BitTask> task_ptr =
            BitService::controller->GetTask(info_hash);
        if (!task_ptr)
            return false;

        std::tr1::shared_ptr<BitPeerConnection> shared_this = shared_from_this();

        assert(owner_);
        owner_->LetMeLeave(shared_this);

        bitdata_ = task_ptr->GetBitData();
        task_ptr->AttachPeer(shared_this);
        return true;
    }

    void BitPeerConnection::PreparePeerData(const std::string& peer_id)
    {
        peer_data_.reset(new BitPeerData(peer_id));
        bitdata_->AddPeerData(peer_data_);
    }

    void BitPeerConnection::DropConnection()
    {
        ClearNetProcessor();
        owner_->LetMeLeave(shared_from_this());
    }

    void BitPeerConnection::SendHandshake()
    {
        Buffer buffer = net_processor_->GetBuffer(handshake_size);

        char *data = buffer.GetBuffer();
        memset(data, 0, handshake_size);
        *data++ = protocol_string_len;

        memcpy(data, protocol_string, protocol_string_len);
        data += protocol_string_len + protocol_reserved;

        Sha1Value info_hash = NetByteOrder(bitdata_->GetInfoHash());
        memcpy(data, info_hash.GetData(), info_hash.GetDataSize());
        data += info_hash.GetDataSize();

        std::string peer_id = bitdata_->GetPeerId();
        memcpy(data, peer_id.c_str(), peer_id.size());

        net_processor_->Send(buffer);
    }

    void BitPeerConnection::MarkPeerHavePiece(int piece_index)
    {
    }

} // namespace core
} // namespace bittorrent

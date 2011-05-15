#include "BitPeerConnection.h"
#include "BitData.h"
#include "BitCache.h"
#include "BitPeerData.h"
#include "BitService.h"
#include "BitController.h"
#include "BitDownloadDispatcher.h"
#include "../net/NetHelper.h"
#include "../sha1/NetSha1Value.h"
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

    void ParseRequestData(const char *data, int *index, int *begin, int *length)
    {
        const int *net_int = reinterpret_cast<const int *>(data);
        *index = bittorrent::net::NetToHosti(*net_int++);
        *begin = bittorrent::net::NetToHosti(*net_int++);
        *length = bittorrent::net::NetToHosti(*net_int);
    }

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
          request_timeouter_(socket.GetService()),
          net_processor_(new NetProcessor(socket))
    {
        assert(owner_);
        BindNetProcessorCallbacks();
        InitTimers();
    }

    BitPeerConnection::BitPeerConnection(const std::shared_ptr<BitData>& bitdata,
                                         net::IoService& io_service,
                                         PeerConnectionOwner *owner)
        : owner_(owner),
          request_timeouter_(io_service),
          bitdata_(bitdata),
          net_processor_(new NetProcessor(io_service))
    {
        assert(owner_);
        BindNetProcessorCallbacks();
    }

    BitPeerConnection::~BitPeerConnection()
    {
        ClearTimers();
        ClearNetProcessor();
        if (bitdata_) bitdata_->DelPeerData(peer_data_);
    }

    // static
    void BitPeerConnection::CompleteRead(const std::tr1::weak_ptr<BitPeerConnection>& conn,
                                         int index, int begin, int length,
                                         bool read_ok, const char *block)
    {
        if (conn.expired())
            return ;

        std::tr1::shared_ptr<BitPeerConnection> peer = conn.lock();
        if (read_ok)
            peer->SendPiece(index, begin, length, block);
        peer->peer_request_.DelRequest(index, begin, length);
    }

    void BitPeerConnection::Connect(const net::Address& remote_address,
                                    const net::Port& remote_listen_port)
    {
        net_processor_->Connect(remote_address, remote_listen_port);
    }

    void BitPeerConnection::Receive()
    {
        net_processor_->Receive();
    }

    void BitPeerConnection::SetOwner(PeerConnectionOwner *owner)
    {
        assert(owner);
        owner_ = owner;
    }

    void BitPeerConnection::SetInterested(bool interested)
    {
        SendNoPayloadMessage(interested ? INTERESTED : NOT_INTERESTED);
        connection_state_.am_interested = interested;
    }

    void BitPeerConnection::SetChoke(bool choke)
    {
        SendNoPayloadMessage(choke ? CHOKE : UNCHOKE);
        connection_state_.am_choking = choke;
    }

    void BitPeerConnection::HavePiece(std::size_t piece_index)
    {
        // handshake is not complete, we do not send HAVE message
        if (!peer_data_)
            return ;
        SendHave(piece_index);
    }

    void BitPeerConnection::Complete()
    {
        // handshake is not complete, we do nothing
        if (!peer_data_)
            return ;
        SetInterested(false);
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
        InitTimers();
        SendHandshake();
    }

    void BitPeerConnection::ConnectClosed()
    {
        DropConnection();
    }

    void BitPeerConnection::ProcessProtocol(const char *data, std::size_t size)
    {
        assert(data);
        // connection is drop
        if (!net_processor_)
            return ;

        // reset disconnect timer
        SetDisconnectTimer();

        if (size == handshake_size && ProcessHandshake(data))
            return ;

        // connection is drop in ProcessHandshake
        if (!net_processor_)
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
        OnHandshake();
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
        // we just do nothing here, because any protocol will be
        // processed in ProcessProtocol, and in that function will
        // keep the connection alive automatically
    }

    void BitPeerConnection::ProcessChoke(bool choke)
    {
        connection_state_.peer_choking = choke;
        if (!choke)
            RequestPieceBlock();
    }

    void BitPeerConnection::ProcessInterested(bool interested)
    {
        connection_state_.peer_interested = interested;
        if (interested)
            SetChoke(false);
    }

    void BitPeerConnection::ProcessHave(const char *data, std::size_t len)
    {
        if (len != sizeof(int) || !peer_data_)
        {
            DropConnection();
            return ;
        }

        int piece_index = net::NetToHosti(*reinterpret_cast<const int *>(data));
        peer_data_->PeerHavePiece(piece_index);
        RequestPieceBlock();
    }

    void BitPeerConnection::ProcessBitfield(const char *data, std::size_t len)
    {
        if (!peer_data_ || !peer_data_->SetPeerBitfield(data, len))
        {
            DropConnection();
            return ;
        }

        RequestPieceBlock();
    }

    void BitPeerConnection::ProcessRequest(const char *data, std::size_t len)
    {
        if (len != 3 * sizeof(int))
        {
            DropConnection();
            return ;
        }

        // too many request, we do not accept
        if (peer_request_.Size() >= 5)
            return ;

        int index = 0;
        int begin = 0;
        int length = 0;
        ParseRequestData(data, &index, &begin, &length);
        peer_request_.AddRequest(index, begin, length);

        cache_->Read(index, begin, length,
                std::tr1::bind(&BitPeerConnection::CompleteRead,
                    std::tr1::weak_ptr<BitPeerConnection>(shared_from_this()),
                        index, begin, length, std::tr1::placeholders::_1,
                            std::tr1::placeholders::_2));
    }

    void BitPeerConnection::ProcessPiece(const char *data, std::size_t len)
    {
        if (len <= 2 * sizeof(int))
        {
            DropConnection();
            return ;
        }

        const int *net_int = reinterpret_cast<const int *>(data);
        int index = net::NetToHosti(*net_int++);
        int begin = net::NetToHosti(*net_int);
        int length = len - 2 * sizeof(int);
        data += 2 * sizeof(int);

        bitdata_->IncreaseCurrentDownload(length);

        BitRequestList::Iterator it = requesting_list_.FindRequest(index, begin, length);
        if (it != requesting_list_.End())
        {
            request_timeouter_.CancelTimeOut(it);
            requesting_list_.Erase(it);
            cache_->Write(index, begin, length, data);
            RequestPieceBlock();
        }
    }

    void BitPeerConnection::ProcessCancel(const char *data, std::size_t len)
    {
        if (len != 3 * sizeof(int))
        {
            DropConnection();
            return ;
        }

        int index = 0;
        int begin = 0;
        int length = 0;
        ParseRequestData(data, &index, &begin, &length);
        peer_request_.DelRequest(index, begin, length);
    }

    bool BitPeerConnection::AttachTask(const Sha1Value& info_hash)
    {
        assert(BitService::controller);
        PeerConnectionOwner *old_owner = owner_;

        std::tr1::shared_ptr<BitPeerConnection> shared_this = shared_from_this();
        bool attach = BitService::controller->AttachPeerToTask(info_hash, shared_this, &bitdata_);

        if (attach)
            old_owner->LetMeLeave(shared_this);

        return attach;
    }

    void BitPeerConnection::PreparePeerData(const std::string& peer_id)
    {
        peer_data_.reset(new BitPeerData(peer_id, bitdata_->GetPieceCount()));
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
        memcpy(data, peer_id.data(), peer_id.size());

        net_processor_->Send(buffer);
        SetKeepAliveTimer();
    }

    void BitPeerConnection::SendKeepAlive()
    {
        int length_prefix = net::HostToNeti(0);
        net_processor_->Send(
            reinterpret_cast<char *>(&length_prefix), sizeof(length_prefix));
        SetKeepAliveTimer();
    }

    void BitPeerConnection::SendNoPayloadMessage(char id)
    {
        Buffer buffer = net_processor_->GetBuffer(sizeof(int) + sizeof(char));
        char *data = buffer.GetBuffer();
        *reinterpret_cast<int *>(data) = net::HostToNeti(1);
        *(data + sizeof(int)) = id;
        net_processor_->Send(buffer);
        SetKeepAliveTimer();
    }

    void BitPeerConnection::SendHave(int piece_index)
    {
        Buffer buffer = net_processor_->GetBuffer(2 * sizeof(int) + sizeof(char));
        char *data = buffer.GetBuffer();
        *reinterpret_cast<int *>(data) = net::HostToNeti(5);
        data += sizeof(int);
        *data++ = HAVE;
        *reinterpret_cast<int *>(data) = net::HostToNeti(piece_index);
        net_processor_->Send(buffer);
        SetKeepAliveTimer();
    }

    void BitPeerConnection::SendBitfield()
    {
        const BitPieceMap& map = bitdata_->GetPieceMap();
        std::size_t size = map.GetMapSize();

        Buffer buffer = net_processor_->GetBuffer(sizeof(int) + sizeof(char) + size);
        char *data = buffer.GetBuffer();
        int length_prefix = sizeof(char) + size;
        *reinterpret_cast<int *>(data) = net::HostToNeti(length_prefix);
        data += sizeof(int);
        *data++ = BITFIELD;
        map.ToBitfield(data);
        net_processor_->Send(buffer);
        SetKeepAliveTimer();
    }

    void BitPeerConnection::SendRequest(int index, int begin, int length)
    {
        Buffer buffer = net_processor_->GetBuffer(4 * sizeof(int) + sizeof(char));
        char *data = buffer.GetBuffer();
        int length_prefix = 3 * sizeof(int) + sizeof(char);
        *reinterpret_cast<int *>(data) = net::HostToNeti(length_prefix);
        data += sizeof(int);
        *data++ = REQUEST;

        int *net_int = reinterpret_cast<int *>(data);
        *net_int++ = net::HostToNeti(index);
        *net_int++ = net::HostToNeti(begin);
        *net_int = net::HostToNeti(length);
        net_processor_->Send(buffer);
        SetKeepAliveTimer();
    }

    void BitPeerConnection::SendPiece(int index, int begin, int length, const char *block)
    {
        Buffer buffer = net_processor_->GetBuffer(3 * sizeof(int) + sizeof(char) + length);
        char *data = buffer.GetBuffer();
        int length_prefix = 2 * sizeof(int) + sizeof(char) + length;
        *reinterpret_cast<int *>(data) = net::HostToNeti(length_prefix);
        data += sizeof(int);
        *data++ = PIECE;

        int *net_int = reinterpret_cast<int *>(data);
        *net_int++ = net::HostToNeti(index);
        *net_int = net::HostToNeti(begin);
        data += 2 * sizeof(int);
        memcpy(data, block, length);
        net_processor_->Send(buffer);
        SetKeepAliveTimer();

        bitdata_->IncreaseUploaded(length);
    }

    void BitPeerConnection::OnHandshake()
    {
        assert(BitService::controller);
        cache_ = BitService::controller->GetTaskCache(bitdata_->GetInfoHash());
        assert(cache_);
        download_dispatcher_ =
            BitService::controller->GetTaskDownloadDispather(bitdata_->GetInfoHash());
        assert(download_dispatcher_);

        SendBitfield();

        if (!bitdata_->IsDownloadComplete())
            SetInterested(true);
    }

    void BitPeerConnection::RequestPieceBlock()
    {
        if (!connection_state_.am_interested ||
            connection_state_.peer_choking)
            return ;

        std::size_t requesting_count = requesting_list_.Size();
        if (requesting_count >= 3)
            return ;

        if (wait_request_.Empty())
            download_dispatcher_->DispatchRequestList(peer_data_, wait_request_);

        std::size_t max_request = 3 - requesting_count;
        std::size_t count = 0;
        BitRequestList::Iterator it = wait_request_.Begin();
        while (count < max_request && it != wait_request_.End())
        {
            BitRequestList::Iterator it_of_requesting =
                requesting_list_.Splice(wait_request_, it++);
            PostRequest(it_of_requesting);
            ++count;
        }
    }

    void BitPeerConnection::PostRequest(BitRequestList::Iterator it)
    {
        SendRequest(it->index, it->begin, it->length);
        request_timeouter_.ApplyTimeOut(it,
                std::tr1::bind(&BitPeerConnection::RequestTimeOut, this, it));
    }

    void BitPeerConnection::RequestTimeOut(BitRequestList::Iterator it)
    {
        request_timeouter_.CancelTimeOut(it);
        download_dispatcher_->ReturnRequest(requesting_list_, it);
    }

    void BitPeerConnection::InitTimers()
    {
        keep_alive_timer_.SetCallback(
                std::tr1::bind(&BitPeerConnection::SendKeepAlive, this));
        disconnect_timer_.SetCallback(
                std::tr1::bind(&BitPeerConnection::DropConnection, this));
        request_timeouter_.AddToTimerService(&keep_alive_timer_);
        request_timeouter_.AddToTimerService(&disconnect_timer_);
        SetKeepAliveTimer();
        SetDisconnectTimer();
    }

    void BitPeerConnection::SetKeepAliveTimer()
    {
        const int keep_alive_time = 2 * 60 * 1000;
        keep_alive_timer_.SetDeadline(keep_alive_time);
    }

    void BitPeerConnection::SetDisconnectTimer()
    {
        const int disconnect_time = 3 * 60 * 1000;
        disconnect_timer_.SetDeadline(disconnect_time);
    }

    void BitPeerConnection::ClearTimers()
    {
        request_timeouter_.RemoveFromTimerService(&keep_alive_timer_);
        request_timeouter_.RemoveFromTimerService(&disconnect_timer_);
    }

} // namespace core
} // namespace bittorrent

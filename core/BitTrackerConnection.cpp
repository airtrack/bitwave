#include "BitTrackerConnection.h"
#include "BitService.h"
#include "bencode/TrackerResponse.h"
#include "../net/Address.h"
#include "../net/TimerService.h"
#include "../protocol/URI.h"
#include "../protocol/Request.h"
#include "../sha1/NetSha1Value.h"
#include <assert.h>
#include <string.h>
#include <vector>
#include <functional>

namespace bittorrent {
namespace core {

    using namespace std::tr1::placeholders;

    // static
    DefaultBufferCache BitTrackerConnection::socket_buffer_cache_;

    BitTrackerConnection::BitTrackerConnection(const std::string& url,
                                               const BitRepository::BitDataPtr& bitdata,
                                               net::IoService& io_service)
        : io_service_(io_service),
          bitdata_(bitdata),
          url_(url),
          connecting_(false),
          need_close_(false)
    {
        http::URI uri(url);
        host_ = uri.GetHost();

        net::ServicePtr<net::ResolveService> address_resolver_ptr(io_service);
        assert(address_resolver_ptr);
        net::ResolveHint hint(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        address_resolver_ptr->AsyncResolve(host_, "", hint,
                std::tr1::bind(
                    &BitTrackerConnection::ResolveHandler,
                    this, _1, _2, _3));

        response_unpacker_.SetUnpackCallback(
                std::tr1::bind(
                    &BitTrackerConnection::ProcessResponse,
                    this, _1, _2));

        reconnect_timer_.SetCallback(
                std::tr1::bind(
                    &BitTrackerConnection::ReconnectTimerCallback,
                    this));
    }

    BitTrackerConnection::~BitTrackerConnection()
    {
        Close();
        CloseReconnectTimer();
    }

    void BitTrackerConnection::UpdateTrackerInfo()
    {
        if (connecting_)
            return ;
        ConnectTracker();
    }

    void BitTrackerConnection::ResolveHandler(const std::string& nodename,
                                              const std::string& servname,
                                              const net::ResolveResult& result)
    {
        assert(nodename == host_);
        host_address_ = result;

        ConnectTracker();
    }

    void BitTrackerConnection::ConnectTracker()
    {
        sockaddr *addr = 0;
        net::ResolveResult::iterator it = host_address_.begin();
        if (it != host_address_.end())
            addr = it->ai_addr;

        if (addr)
        {
            net::Port port(80);
            net::Address address(reinterpret_cast<sockaddr_in *>(addr));
            socket_.Reset(new net::AsyncSocket(io_service_));
            socket_->AsyncConnect(address, port,
                    std::tr1::bind(
                        &BitTrackerConnection::ConnectHandler,
                        this, _1));
            connecting_ = true;
        }
    }

    void BitTrackerConnection::SendRequest()
    {
        http::URI uri(url_);

        Sha1Value net_sha1 = NetByteOrder(bitdata_->GetInfoHash());
        std::string peer_id = bitdata_->GetPeerId();
        short listen_port = BitService::repository->GetListenPort();
        long long uploaded = bitdata_->GetUploaded();
        long long downloaded = bitdata_->GetDownloaded();
        long long left = bitdata_->GetTotalSize() - downloaded;

        uri.AddQuery("info_hash", net_sha1.GetData(), net_sha1.GetDataSize());
        uri.AddQuery("peer_id", peer_id.c_str(), 20);
        uri.AddQuery("port", listen_port);
        uri.AddQuery("uploaded", uploaded);
        uri.AddQuery("downloaded", downloaded);
        uri.AddQuery("left", left);
        uri.AddQuery("compact", 1);
        uri.AddQuery("numwant", 200);

        http::Request request(uri);
        std::string request_text = request.GetRequestText();

        request_buffer_ = socket_buffer_cache_.GetBuffer(request_text.size());
        memcpy(request_buffer_.GetBuffer(), request_text.c_str(), request_text.size());

        socket_->AsyncSend(request_buffer_,
                std::tr1::bind(
                    &BitTrackerConnection::SendHandler,
                    this, _1, _2));
    }

    void BitTrackerConnection::ReceiveResponse()
    {
        response_buffer_ = socket_buffer_cache_.GetBuffer(2048);
        socket_->AsyncReceive(response_buffer_,
                std::tr1::bind(
                    &BitTrackerConnection::ReceiveHandler,
                    this, _1, _2));
    }

    void BitTrackerConnection::Close()
    {
        response_unpacker_.Clear();
        socket_.Reset();

        if (request_buffer_)
            socket_buffer_cache_.FreeBuffer(request_buffer_);
        if (response_buffer_)
            socket_buffer_cache_.FreeBuffer(response_buffer_);
        connecting_ = false;
        need_close_ = false;
    }

    void BitTrackerConnection::ConnectHandler(bool connected)
    {
        if (!connecting_)
            return ;

        if (connected)
        {
            SendRequest();
            ReceiveResponse();
        }
        else
        {
            Close();
        }
    }

    void BitTrackerConnection::SendHandler(bool success, int send)
    {
        if (!connecting_)
            return ;

        if (success)
            socket_buffer_cache_.FreeBuffer(request_buffer_);
        else
            Close();
    }

    void BitTrackerConnection::ReceiveHandler(bool success, int received)
    {
        if (!connecting_)
            return ;

        if (success)
        {
            response_unpacker_.StreamDataArrive(
                    response_buffer_.GetBuffer(), received);
            if (need_close_)
            {
                Close();
            }
            else
            {
                socket_buffer_cache_.FreeBuffer(response_buffer_);
                ReceiveResponse();
            }
        }
        else
        {
            Close();
        }
    }

    void BitTrackerConnection::ProcessResponse(const char *data, std::size_t size)
    {
        try
        {
            http::Response response(data, size);
            if (response.GetStatusCodeType() == http::Response::SUCCESS)
            {
                std::size_t content_size = response.GetContentSize();
                if (content_size > 0)
                {
                    const char * content = response.GetContentBufPointer();
                    bentypes::TrackerResponse response_info(content, content_size);

                    if (!response_info.IsFailure())
                    {
                        typedef std::vector<bentypes::TrackerResponse::PeerInfo> PeersInfo;
                        PeersInfo peers_info;
                        response_info.GetPeerInfo(peers_info);

                        for (PeersInfo::iterator it = peers_info.begin();
                                it != peers_info.end(); ++it)
                        {
                            BitData::PeerKey key(it->ip, it->port);
                            bitdata_->AddPeerData(key);
                        }

                        int interval = response_info.GetInterval();
                        StartReconnectTimer(interval);
                    }
                }
            }
        }
        catch (...)
        {
            // we do nothing here ...
        }

        need_close_ = true;
    }

    void BitTrackerConnection::StartReconnectTimer(int seconds)
    {
        assert(seconds > 0);
        net::ServicePtr<net::TimerService> timer_service(io_service_);
        assert(timer_service);
        reconnect_timer_.SetDeadline(seconds * 1000);
        timer_service->AddTimer(&reconnect_timer_);
    }

    void BitTrackerConnection::ReconnectTimerCallback()
    {
        CloseReconnectTimer();
        ConnectTracker();
    }

    void BitTrackerConnection::CloseReconnectTimer()
    {
        net::ServicePtr<net::TimerService> timer_service(io_service_);
        assert(timer_service);
        timer_service->DelTimer(&reconnect_timer_);
    }

} // namespace core
} // namespace bittorrent

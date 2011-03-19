#include "BitTrackerConnection.h"
#include "BitData.h"
#include "BitService.h"
#include "BitRepository.h"
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

    BitTrackerConnection::BitTrackerConnection(const std::string& url,
                                               const std::tr1::shared_ptr<BitData>& bitdata,
                                               net::IoService& io_service)
        : io_service_(io_service),
          reconnect_interval_(30),
          bitdata_(bitdata),
          url_(url)
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

        reconnect_timer_.SetCallback(
                std::tr1::bind(
                    &BitTrackerConnection::ReconnectTimerCallback,
                    this));
    }

    BitTrackerConnection::~BitTrackerConnection()
    {
        ClearNetProcessor();
        CloseReconnectTimer();
    }

    void BitTrackerConnection::UpdateTrackerInfo()
    {
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
        if (net_processor_)
            return ;

        sockaddr *addr = 0;
        net::ResolveResult::iterator it = host_address_.begin();
        if (it != host_address_.end())
            addr = it->ai_addr;

        if (addr)
        {
            net_processor_.reset(new NetProcessor(io_service_));

            net_processor_->SetProtocolCallback(
                    std::tr1::bind(&BitTrackerConnection::ProcessResponse, this, _1, _2));
            net_processor_->SetConnectCallback(
                    std::tr1::bind(&BitTrackerConnection::OnTrackerConnect, this));
            net_processor_->SetDisconnectCallback(
                    std::tr1::bind(&BitTrackerConnection::OnTrackerDisconnect, this));

            net::Port port(80);
            net::Address address(reinterpret_cast<sockaddr_in *>(addr));
            net_processor_->Connect(address, port);
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
        assert(net_processor_);
        net_processor_->Send(request_text.c_str(), request_text.size());
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
                            bitdata_->AddPeerListenInfo(it->ip, it->port);
                        }

                        reconnect_interval_ = response_info.GetInterval();
                    }
                }
            }
        }
        catch (...)
        {
            // we do nothing here ...
        }

        net_processor_->Close();
    }

    void BitTrackerConnection::OnTrackerConnect()
    {
        SendRequest();
    }

    void BitTrackerConnection::OnTrackerDisconnect()
    {
        ClearNetProcessor();
        StartReconnectTimer(reconnect_interval_);
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

    void BitTrackerConnection::ClearNetProcessor()
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

} // namespace core
} // namespace bittorrent

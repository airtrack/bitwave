#ifndef BIT_TRACKER_CONNECTION_H
#define BIT_TRACKER_CONNECTION_H

#include "BitData.h"
#include "BitRepository.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../buffer/Buffer.h"
#include "../net/IoService.h"
#include "../net/ResolveService.h"
#include "../net/StreamUnPacker.h"
#include "../protocol/Response.h"
#include "../timer/Timer.h"
#include <string>

namespace bittorrent {
namespace core {

    class BitTrackerConnection : private NotCopyable
    {
    public:
        BitTrackerConnection(const std::string& url,
                             const BitRepository::BitDataPtr& bitdata,
                             net::IoService& io_service);

        ~BitTrackerConnection();

        void UpdateTrackerInfo();

    private:
        typedef net::StreamUnpacker<http::ResponseUnpackRuler> ResponseUnpacker;

        void ResolveHandler(const std::string& nodename,
                            const std::string& servname,
                            const net::ResolveResult& result);
        void ConnectTracker();
        void SendRequest();
        void ReceiveResponse();
        void Close();
        void ConnectHandler(bool connected);
        void SendHandler(bool success, int send);
        void ReceiveHandler(bool success, int received);
        void ProcessResponse(const char *data, std::size_t size);
        void StartReconnectTimer(int seconds);
        void ReconnectTimerCallback();
        void CloseReconnectTimer();

        static DefaultBufferCache socket_buffer_cache_;

        net::IoService& io_service_;
        ScopePtr<net::AsyncSocket> socket_;
        net::ResolveResult host_address_;
        ResponseUnpacker response_unpacker_;
        Timer reconnect_timer_;

        Buffer request_buffer_;
        Buffer response_buffer_;

        BitRepository::BitDataPtr bitdata_;
        std::string url_;
        std::string host_;
        bool connecting_;
        bool need_close_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_TRACKER_CONNECTION_H

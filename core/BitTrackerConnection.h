#ifndef BIT_TRACKER_CONNECTION_H
#define BIT_TRACKER_CONNECTION_H

#include "BitNetProcessor.h"
#include "../base/BaseTypes.h"
#include "../net/ResolveService.h"
#include "../protocol/Response.h"
#include "../timer/Timer.h"
#include <string>
#include <memory>

namespace bittorrent {
namespace core {

    class BitData;

    class BitTrackerConnection : private NotCopyable
    {
    public:
        BitTrackerConnection(const std::string& url,
                             const std::tr1::shared_ptr<BitData>& bitdata,
                             net::IoService& io_service);

        ~BitTrackerConnection();

        void UpdateTrackerInfo();

    private:
        typedef BitNetProcessor<http::ResponseUnpackRuler> NetProcessor;

        void ResolveHandler(const std::string& nodename,
                            const std::string& servname,
                            const net::ResolveResult& result);
        void ConnectTracker();
        void SendRequest();
        void ProcessResponse(const char *data, std::size_t size);
        void OnTrackerConnect();
        void OnTrackerDisconnect();
        void StartReconnectTimer(int seconds);
        void ReconnectTimerCallback();
        void CloseReconnectTimer();
        void ClearNetProcessor();

        net::IoService& io_service_;
        net::ResolveResult host_address_;
        std::tr1::shared_ptr<NetProcessor> net_processor_;
        Timer reconnect_timer_;
        int reconnect_interval_;

        std::tr1::shared_ptr<BitData> bitdata_;
        std::string url_;
        std::string host_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_TRACKER_CONNECTION_H

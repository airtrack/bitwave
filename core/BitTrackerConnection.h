#ifndef BIT_TRACKER_CONNECTION_H
#define BIT_TRACKER_CONNECTION_H

#include "BitData.h"
#include "BitRepository.h"
#include "../base/BaseTypes.h"
#include "../buffer/Buffer.h"
#include "../net/IoService.h"
#include "../net/ResolveService.h"
#include "../net/StreamUnPacker.h"
#include "../protocol/Response.h"
#include <string>

namespace bittorrent {
namespace core {

    class BitTrackerConnection : private NotCopyable
    {
    public:
        BitTrackerConnection(const std::string& url,
                             const BitRepository::BitDataPtr& bitdata,
                             net::IoService& io_service,
                             net::ResolveService& resolve_service);

        ~BitTrackerConnection();

        void UpdateTrackerInfo();

    private:
        void ResolveHandler(const std::string& nodename,
                            const std::string& servname,
                            const net::ResolveResult& result);
        void ConnectTracker();
        void ConnectHandler(bool connected);
        void SendHandler(bool success, int send);
        void ReceiveHandler(bool success, int received);
        void Close();
        void SendRequest();
        void ReceiveResponse();
        void ProcessResponse(const char *data, std::size_t size);

        typedef net::StreamUnpacker<http::ResponseUnpackRuler> ResponseUnpacker;

        // socket buffer
        static DefaultBufferCache socket_buffer_cache_;

        net::IoService& io_service_;
        net::SocketHandler *socket_handler_;
        net::ResolveResult host_address_;
        ResponseUnpacker response_unpacker_;

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

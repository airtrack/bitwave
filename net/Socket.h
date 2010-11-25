#ifndef SOCKET_H
#define SOCKET_H

#include "Address.h"

namespace bittorrent {
namespace net {

    template<typename ImplementType, typename ServiceType>
    class Socket
    {
    public:
        typedef ImplementType implement_type;
        typedef ServiceType service_type;

        explicit Socket(service_type& service)
            : service_(service),
              implement_(service)
        {
        }

        Socket(service_type& service, const implement_type& implement)
            : service_(service),
              implement_(implement)
        {
        }

        template<typename Handler>
        void AsyncConnect(const Address& address, const Port& port, Handler handler)
        {
            service_.AsyncConnect(implement_, address, port, handler);
        }

        template<typename Buffer, typename Handler>
        void AsyncReceive(Buffer& buffer, Handler handler)
        {
            service_.AsyncReceive(implement_, buffer, handler);
        }

        template<typename Buffer, typename Handler>
        void AsyncSend(const Buffer& buffer, Handler handler)
        {
            service_.AsyncSend(implement_, buffer, handler);
        }

        void Close()
        {
            implement_.Close();
        }

    private:
        service_type& service_;
        implement_type implement_;
    };

    template<typename ImplementType, typename ServiceType>
    class Listener
    {
    public:
        typedef ImplementType implement_type;
        typedef ServiceType service_type;

        Listener(const Address& address, const Port& port, service_type& service)
            : address_(address),
              port_(port),
              service_(service),
              implement_(service)
        {
        }

        template<typename Handler>
        void AsyncAccept(Handler handler)
        {
            service.AsyncAccept(implement_, handler);
        }

        void Close()
        {
            implement_.Close();
        }

    private:
        Address address_;
        Port port_;
        service_type& service;
        implement_type implement_;
    };

} // namespace net
} // namespace bittorrent

#endif // SOCKET_H

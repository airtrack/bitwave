#ifndef SOCKET_H
#define SOCKET_H

#include "Address.h"
#include "Exception.h"

namespace bittorrent {
namespace net {

    // a template class associate socket implement with service
    // and provide async-operations for socket implement by service
    template<typename ImplementType, typename ServiceType>
    class Socket
    {
    public:
        typedef ImplementType implement_type;
        typedef ServiceType service_type;

        // construct a new socket associate with service
        explicit Socket(service_type& service)
            : service_(service),
              implement_(service)
        {
        }

        // construct a socket handler associate with service and exist socket
        Socket(service_type& service, const implement_type& implement)
            : service_(service),
              implement_(implement)
        {
        }

        template<typename Handler>
        void AsyncConnect(const Address& address, const Port& port, const Handler& handler)
        {
            service_.AsyncConnect(implement_, address, port, handler);
        }

        template<typename Buffer, typename Handler>
        void AsyncReceive(Buffer& buffer, const Handler& handler)
        {
            service_.AsyncReceive(implement_, buffer, handler);
        }

        template<typename Buffer, typename Handler>
        void AsyncSend(const Buffer& buffer, const Handler& handler)
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

    // a template Listener class associate socket implement with service
    // and provide AsyncAccept operation for socket implement by service
    template<typename ImplementType, typename ServiceType>
    class Listener
    {
    public:
        typedef ImplementType implement_type;
        typedef ServiceType service_type;

        // construct a Listener associate with service and listen address, port
        Listener(const Address& address, const Port& port, service_type& service)
            : address_(address),
              port_(port),
              service_(service),
              implement_(service)
        {
            sockaddr_in name = Ipv4Address(address_, port_);
            int result = ::bind(implement_.Get(), (sockaddr *)&name, sizeof(name));
            if (result == SOCKET_ERROR)
                throw NetException(BIND_LISTENER_SOCKET_ERROR);

            result = ::listen(implement_.Get(), SOMAXCONN);
            if (result == SOCKET_ERROR)
                throw NetException(LISTEN_LISTENER_SOCKET_ERROR);
        }

        template<typename Handler>
        void AsyncAccept(const Handler& handler)
        {
            service_.AsyncAccept(implement_, handler);
        }

        void Close()
        {
            implement_.Close();
        }

    private:
        Address address_;
        Port port_;
        service_type& service_;
        implement_type implement_;
    };

} // namespace net
} // namespace bittorrent

#endif // SOCKET_H

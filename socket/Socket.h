#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "Address.h"
#include "IoService.h"

namespace bittorrent
{
    class SocketHandler
    {
    public:
        explicit SocketHandler(IoService& service)
            : service_(service),
              sock_(service.GetSocket())
        {
        }

        SocketHandler(SOCKET sock, IoService& service)
            : service_(service),
              sock_(sock)
        {
        }

        template<typename DataBuffer, typename SendHandler>
        void AsyncSend(DataBuffer& buffer, SendHandler sendhandler)
        {
            service_.AsyncSend(sock_, buffer, sendhandler);
        }

        template<typename DataBuffer, typename RecvHandler>
        void AsyncRecv(DataBuffer& buffer, RecvHandler recvhandler)
        {
            service_.AsyncRecv(sock_, buffer, recvhandler);
        }

        template<typename ConnectHandler>
        void AsyncConn(Address address, Port port, ConnectHandler connhandler)
        {
            service_.AsyncConn(sock_, address, port, connhandler);
        }

        IoService& Service() const
        {
            return service_;
        }

        void Close()
        {
            service_.FreeSocket(sock_);
            sock_ = INVALID_SOCKET;
        }

    private:
        IoService& service_;
        SOCKET sock_;
    };

    class AcceptorHandler
    {
    public:
        AcceptorHandler(IoService& service, Port port, Address address = Address())
            : service_(service),
              sock_(service.GetSocket())
        {
            sockaddr_in listenaddr;
            listenaddr.sin_family = AF_INET;
            listenaddr.sin_port = port;
            listenaddr.sin_addr.s_addr = address;

            try
            {
                if (bind(sock_, (sockaddr *)&listenaddr, sizeof(listenaddr)))
                    throw "";
                if (listen(sock_, SOMAXCONN))
                    throw "";
            } catch (...)
            {
                Close();
                throw;
            }
        }

        AcceptorHandler(SOCKET sock, IoService& service)
            : service_(service),
              sock_(sock)
        {
        }

        template<typename AcceptHandler>
        void AsyncAccept(AcceptHandler accepthandler)
        {
            service_.AsyncAccept(sock_, accepthandler);
        }

        IoService& Service() const
        {
            return service_;
        }

        void Close()
        {
            service_.FreeSocket(sock_);
            sock_ = INVALID_SOCKET;
        }

    private:
        IoService& service_;
        SOCKET sock_;
    };
} // namespace bittorrent

#endif // _SOCKET_H_

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
        explicit AcceptorHandler(IoService& service)
            : service_(service),
              sock_(service.GetSocket())
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
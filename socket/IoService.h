#ifndef _IO_SERVICE_H_
#define _IO_SERVICE_H_

#include "Address.h"
#include "SocketManager.h"
#include "../base/BaseTypes.h"

namespace bittorrent
{
    class IoService : private NotCopyable
    {
        // this class manager all sockets are overlapped at this io service
        class IoSocketedManager
        {
        public:
            IoSocketedManager();

            void FreeSocket(SOCKET sock);
        };

    public:
        IoService();

        template<typename DataBuffer, typename SendHandler>
        void AsyncSend(SOCKET sock, DataBuffer buffer, SendHandler sendhandler);

        template<typename DataBuffer, typename RecvHandler>
        void AsyncRecv(SOCKET sock, DataBuffer buffer, RecvHandler recvhandler);

        template<typename ConnectHandler>
        void AsyncConn(SOCKET sock, Address address, Port port, ConnectHandler connhandler);

        template<typename AcceptHandler>
        void AsyncAccept(SOCKET sock, AcceptHandler accepthandler);

        void Run();
        SOCKET GetSocket()
        {
            return socketmanager_.NewSocket();
        }

        void FreeSocket(SOCKET sock)
        {
            iosocketedmanager_.FreeSocket(sock);
            socketmanager_.FreeSocket(sock);
        }

    private:
        SocketManager socketmanager_;
        IoSocketedManager iosocketedmanager_;
    };
} // namespace bittorrent

#endif // _IO_SERVICE_H_
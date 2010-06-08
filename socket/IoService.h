#ifndef _IO_SERVICE_H_
#define _IO_SERVICE_H_

#include <WinSock2.h>
#include "../base/BaseTypes.h"
#include "IocpData.h"

namespace bittorrent
{
    namespace socket
    {
        class IoException : public BaseException
        {
        public:
            IoException(const char *w)
                : BaseException(w)
            {
            }
        };

        class Buffer;
        class Socket;
        class Acceptor;

        class IoService : private NotCopyable
        {
        public:
            IoService();
            void AddClient(SOCKET sock, const sockaddr_in& addr);

            void Send(Socket *socket, Buffer& buf);
            void Recv(Socket *socket, Buffer& buf);
            void Connect(Socket *socket, const sockaddr *name, int namelen);
            void Accept(Acceptor *acceptor, Socket& socket);

        private:
            HANDLE handle_;
            IocpData iocpdata_;
            unsigned long flag_; // don't care about this value, just used by WSARecv
        };

        inline void BindService(SOCKET sock, const sockaddr_in& addr, IoService *service)
        {
            service->AddClient(sock, addr);
        }
    } // namespace socket
} // namespace bittorrent

#endif // _IO_SERVICE_H_
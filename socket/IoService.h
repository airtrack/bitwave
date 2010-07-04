#ifndef _IO_SERVICE_H_
#define _IO_SERVICE_H_

#include "Address.h"
#include "SocketManager.h"
#include "IocpData.h"
#include "../base/BaseTypes.h"
#include "../thread/Thread.h"

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
        IoService()
            : socketmanager_(),
              iosocketedmanager_(),
              servicehandle_(INVALID_HANDLE_VALUE)
        {
            servicehandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
            if (!servicehandle_) throw "";

            std::size_t threadcount = GetServiceThreadCount();
            for (std::size_t i = 0; i < threadcount; ++i)
                Thread(ServiceThread, (void *)servicehandle_);
        }

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
        static std::size_t GetServiceThreadCount()
        {
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            return sysinfo.dwNumberOfProcessors * 2 + 2;
        }

        static unsigned ServiceThread(void *arg)
        {
            HANDLE servicehandle = (HANDLE)arg;
            unsigned long numofbytes;
            CompletionKey *ck = 0;
            Overlapped *ol = 0;

            while (true)
            {
                if (GetQueuedCompletionStatus(servicehandle, &numofbytes,
                        (PULONG_PTR)&ck, (LPOVERLAPPED *)&ol, INFINITE))
                {
                }
                else
                {
                    if (ol)
                    {
                        // close socket
                    }
                    else
                    {
                        // error
                    }
                }
            }
        }

        SocketManager socketmanager_;
        IoSocketedManager iosocketedmanager_;
        HANDLE servicehandle_;
    };
} // namespace bittorrent

#endif // _IO_SERVICE_H_

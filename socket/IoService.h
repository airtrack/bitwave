#ifndef _IO_SERVICE_H_
#define _IO_SERVICE_H_

#include "Address.h"
#include "SocketManager.h"
#include "IocpData.h"
#include "../base/BaseTypes.h"
#include "../thread/Thread.h"
#include "../thread/Mutex.h"

#include <vector>

namespace bittorrent
{
    // this class is core service class, manage all sockets and io resources
    class IoService : private NotCopyable
    {
        // this class manage all io data of sockets which are overlapped at this io service
        class IoSocketedManager : private NotCopyable
        {
        public:
            explicit IoSocketedManager(IocpData& iocpdata)
                : iocpdata_(iocpdata)
            {
            }

            void AddSocket(SOCKET sock, CompletionKey *ck);
            void BindSocket(SOCKET sock, Overlapped *ol);
            void UnBindSocket(SOCKET sock, Overlapped *ol);
            void FreeSocket(SOCKET sock);

        private:
            IocpData& iocpdata_;
        };

        // this class store all completion io at iocp. then these completed operations
        // need be processed at main thread
        class CompleteOperations : private NotCopyable
        {
        public:
            typedef std::vector<std::pair<CompletionKey *, Overlapped *> > PendingData;
            typedef std::vector<SOCKET> PendingSockets;

            void PendingSendSuccess(CompletionKey *ck, Overlapped *ol);
            void PendingRecvSuccess(CompletionKey *ck, Overlapped *ol);
            void PendingAcceptSuccess(CompletionKey *ck, Overlapped *ol);
            void PendingConnectSuccess(CompletionKey *ck, Overlapped *ol);
            void PendingCloseSocket(SOCKET sock);

            void GetAllSendSuccess(PendingData& data);
            void GetAllRecvSuccess(PendingData& data);
            void GetAllAcceptSuccess(PendingData& data);
            void GetAllConnectSuccess(PendingData& data);
            void GetAllNeedCloseSockets(PendingSockets& sockets);

        private:
            PendingData senddata_;
            PendingData recvdata_;
            PendingData acceptdata_;
            PendingData connectdata_;
            PendingSockets needclosesockets_;

            SpinlocksMutex senddatamutex_;
            SpinlocksMutex recvdatamutex_;
            SpinlocksMutex acceptdatamutex_;
            SpinlocksMutex connectdatamutex_;
            SpinlocksMutex needclosesocketsmutex_;
        };

        // this class store some data of iocp service threads
        struct ServiceThreadLocalData
        {
            HANDLE iocp;
            CompleteOperations& data;

            ServiceThreadLocalData(HANDLE iocphandle, CompleteOperations& co)
                : iocp(iocphandle),
                  data(co)
            {
            }
        };

    public:
        IoService()
            : iocpdata_(),
              socketmanager_(),
              iosocketedmanager_(iocpdata_),
              completeoperations_(),
              servicehandle_(INVALID_HANDLE_VALUE)
        {
            servicehandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
            if (!servicehandle_) throw "";

            std::size_t threadcount = GetServiceThreadCount();
            for (std::size_t i = 0; i < threadcount; ++i)
                Thread(ServiceThread, new ServiceThreadLocalData(servicehandle_, completeoperations_));
        }

        template<typename DataBuffer, typename SendHandler>
        void AsyncSend(SOCKET sock, DataBuffer buffer, SendHandler sendhandler)
        {
            Overlapped *ol = iocpdata_.NewOverlapped();
            ol->ot = SEND;
            ol->buf.buf = buffer.Get();
            ol->buf.len = buffer.Len();
            ol->callback = sendhandler;
            WSASend(sock, &ol->buf, 1, 0, 0, (LPWSAOVERLAPPED)ol, 0);
            iosocketedmanager_.BindSocket(sock, ol);
        }

        template<typename DataBuffer, typename RecvHandler>
        void AsyncRecv(SOCKET sock, DataBuffer buffer, RecvHandler recvhandler)
        {
            DWORD flags = 0;
            Overlapped *ol = iocpdata_.NewOverlapped();
            ol->ot = RECV;
            ol->buf.buf = buffer.Get();
            ol->buf.len = buffer.Len();
            ol->callback = recvhandler;
            WSARecv(sock, &ol->buf, 1, 0, &flags, (LPWSAOVERLAPPED)ol, 0);
            iosocketedmanager_.BindSocket(sock, ol);
        }

        template<typename ConnectHandler>
        void AsyncConn(SOCKET sock, Address address, Port port, ConnectHandler connhandler)
        {
            LPFN_CONNECTEX ConnectEx = 0;
            unsigned long retbytes = 0;
            GUID guid = WSAID_CONNECTEX;

            if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
                        (LPVOID)&guid, sizeof(guid),
                        (LPVOID)&ConnectEx, sizeof(ConnectEx),
                        &retbytes, 0, 0))
                throw "";

            OverLapped *ol = iocpdata_.NewOverlapped();
            ol->ot = CONNECT;
            ol->callback = connhandler;

            sockaddr_in name = Ipv4Address(address, port);
            ConnectEx(sock, &name, sizeof(name), 0, 0, 0, (LPOVERLAPPED)ol);
            iosocketedmanager_.BindSocket(sock, ol);
        }

        template<typename AcceptHandler>
        void AsyncAccept(SOCKET sock, AcceptHandler accepthandler)
        {
            LPFN_ACCEPTEX AcceptEx = 0;
            unsigned long retbytes = 0;
            GUID guid = WSAID_ACCEPTEX;

            if (WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
                        (LPVOID)&guid, sizeof(guid),
                        (LPVOID)&AcceptEx, sizeof(AcceptEx),
                        &retbytes, 0, 0))
                throw "";

            OverLapped *ol = iocpdata_.NewOverlapped();
            ol->ot = ACCEPT;
            ol->accepted = socketmanager_.NewSocket();
            ol->callback = accepthandler;

            AcceptEx(sock, ol->accepted, 0, 0, 0, 0, 0, (LPOVERLAPPED)ol);
            iosocketedmanager_.BindSocket(sock, ol);
        }

        void Run();

        SOCKET GetSocket()
        {
            SOCKET sock = socketmanager_.NewSocket();
            CompletionKey *ck = iocpdata_.NewCompletionKey();
            CreateIoCompletionPort((HANDLE)sock, servicehandle_, (ULONG_PTR)ck, 0);
            iosocketedmanager_.AddSocket(sock, ck);
            return sock;
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
            ScopePtr<ServiceThreadLocalData> ptr(static_cast<ServiceThreadLocalData *>(arg));
            unsigned long numofbytes;
            CompletionKey *ck = 0;
            Overlapped *ol = 0;

            while (true)
            {
                if (GetQueuedCompletionStatus(ptr->iocp, &numofbytes,
                        (PULONG_PTR)&ck, (LPOVERLAPPED *)&ol, INFINITE))
                {
                    switch (ol->ot)
                    {
                        case ACCEPT:
                            ptr->data.PendingAcceptSuccess(ck, ol);
                            break;

                        case CONNECT:
                            ptr->data.PendingConnectSuccess(ck, ol);
                            break;

                        case SEND:
                            ptr->data.PendingSendSuccess(ck, ol);
                            break;

                        case RECV:
                            ptr->data.PendingRecvSuccess(ck, ol);
                            break;
                    }
                }
                else
                {
                    if (ol)
                    {
                        ptr->data.PendingCloseSocket(ck->sock);
                    }
                    else
                    {
                        // error
                        // we do nothing here ...
                    }
                }
            }

            return 0;
        }

        IocpData iocpdata_;
        SocketManager socketmanager_;
        IoSocketedManager iosocketedmanager_;
        CompleteOperations completeoperations_;
        HANDLE servicehandle_;
    };
} // namespace bittorrent

#endif // _IO_SERVICE_H_

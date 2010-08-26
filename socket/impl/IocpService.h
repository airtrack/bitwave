#ifndef _IOCP_SERVICE_H_
#define _IOCP_SERVICE_H_

#include "IocpData.h"

#include "../Address.h"
#include "../Buffer.h"

#include "../../base/BaseTypes.h"
#include "../../base/ScopePtr.h"
#include "../../thread/Thread.h"
#include "../../thread/Mutex.h"

#include <vector>
#include <map>
#include <WinSock2.h>
#include <MSWSock.h>

namespace bittorrent
{
    class CompleteOperations
        : private NotCopyable
    {
    public:
        typedef std::vector<std::pair<CompletionKey *, Overlapped *> > PendingData;
        typedef std::vector<SOCKET> PendingSockets;

        void PendingSendSuccess(CompletionKey *ck, Overlapped *ol)
        {
            LockPendingSuccess(senddata_, senddatamutex_, ck, ol);
        }

        void PendingRecvSuccess(CompletionKey *ck, Overlapped *ol)
        {
            LockPendingSuccess(recvdata_, recvdatamutex_, ck, ol);
        }

        void PendingAcceptSuccess(CompletionKey *ck, Overlapped *ol)
        {
            LockPendingSuccess(acceptdata_, acceptdatamutex_, ck, ol);
        }

        void PendingConnectSuccess(CompletionKey *ck, Overlapped *ol)
        {
            LockPendingSuccess(connectdata_, connectdatamutex_, ck, ol);
        }

        void PendingCloseSocket(SOCKET sock)
        {
            SpinlocksMutexLocker locker(needclosesocketsmutex_);
            needclosesockets_.push_back(sock);
        }

        void GetAllSendSuccess(PendingData& data)
        {
            LockSwapData(senddata_, data, senddatamutex_);
        }

        void GetAllRecvSuccess(PendingData& data)
        {
            LockSwapData(recvdata_, data, recvdatamutex_);
        }

        void GetAllAcceptSuccess(PendingData& data)
        {
            LockSwapData(acceptdata_, data, acceptdatamutex_);
        }

        void GetAllConnectSuccess(PendingData& data)
        {
            LockSwapData(connectdata_, data, connectdatamutex_);
        }

        void GetAllNeedCloseSockets(PendingSockets& sockets)
        {
            LockSwapData(needclosesockets_, sockets, needclosesocketsmutex_);
        }

    private:
        template<typename DataType, typename MutexType>
            void LockSwapData(DataType& data1, DataType& data2, MutexType& mutex)
            {
                typename LockerType<MutexType>::type locker(mutex);
                data1.swap(data2);
            }

        template<typename MutexType>
            void LockPendingSuccess(PendingData& data, MutexType& mutex,
                    CompletionKey *ck, Overlapped *ol)
            {
                typename LockerType<MutexType>::type locker(mutex);
                data.push_back(std::make_pair(ck, ol));
            }

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

    class IocpService
        : private NotCopyable
    {
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
        IocpService();
        ~IocpService();

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
                throw BaseException("can not get ConnectEx function at runtime!");

            Overlapped *ol = iocpdata_.NewOverlapped();
            ol->ot = CONNECT;
            ol->callback = connhandler;

            sockaddr_in local = Ipv4Address(Address(), Port(0));
            int error = ::bind(sock, (sockaddr *)&local, sizeof(local));
            if (error == SOCKET_ERROR)
                throw BaseException("bind connect socket error!");

            sockaddr_in name = Ipv4Address(address, port);
            if (!ConnectEx(sock, (sockaddr *)&name, sizeof(name), 0, 0, 0, (LPOVERLAPPED)ol))
            {
                error = WSAGetLastError();
                if (error != ERROR_IO_PENDING)
                    throw BaseException("call ConnectEx error!");
            }

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
                throw BaseException("can not get AcceptEx function at runtime!");

            Overlapped *ol = iocpdata_.NewOverlapped();
            ol->ot = ACCEPT;
            ol->accepted = GetSocket();
            ol->callback = accepthandler;

            if (!AcceptEx(sock, ol->accepted, acceptaddrbuf_.GetAddrBuf(ol->accepted), 0,
                sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 0, (LPOVERLAPPED)ol))
            {
                if (WSAGetLastError() != ERROR_IO_PENDING)
                    throw BaseException("call AcceptEx error!");
            }

            iosocketedmanager_.BindSocket(sock, ol);
        }

        void Run();
    private:
        // store all AcceptEx function used Address buffer,
        // typically, this class used when IoSerivce as a socket server
        class AcceptAddrBuf
        {
        public:
            char * GetAddrBuf(SOCKET sock);
            void ReleaseAddrBuf(SOCKET sock);

        private:
            static const std::size_t addrbufsize = 2 * (sizeof(sockaddr_in) + 16);

            typedef std::map<SOCKET, char *> SocketAddrBuf;
            SocketAddrBuf socketaddrbuf_;
            DefaultBufferService addrbuf_;
        };

        static std::size_t GetServiceThreadCount();
        static unsigned __stdcall ServiceThread(void *arg);

        void ProcessCompletedSend();
        void ProcessCompletedRecv();
        void ProcessCompletedAccept();
        void ProcessCompletedConnect();
        void ProcessNeedCloseSockets();

        IocpData iocpdata_;
        AcceptAddrBuf acceptaddrbuf_;

        CompleteOperations completeoperations_;
        CompleteOperations::PendingData sendcompletes_;
        CompleteOperations::PendingData recvcompletes_;
        CompleteOperations::PendingData acceptcompletes_;
        CompleteOperations::PendingData connectcompletes_;
        CompleteOperations::PendingSockets needclosesockets_;

        HANDLE servicehandle_;
    };
} // namespace bittorrent

#endif // _IOCP_SERVICE_H_

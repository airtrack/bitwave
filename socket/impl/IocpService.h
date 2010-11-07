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
#include <set>
#include <WinSock2.h>
#include <MSWSock.h>

namespace bittorrent
{
    class SocketHandler;
    class AcceptorHandler;
    class ISocketStream;
    class OSocketStream;

    // the callback prototype of socket which has data to recv
    typedef void (*RecvDataCallback)(SocketHandler&);

    class CompleteOperations : private NotCopyable
    {
    public:
        typedef std::vector<std::pair<CompletionKey *, Overlapped *> > PendingData;

        void PendingSendSuccess(CompletionKey *ck, Overlapped *ol)
        {
            LockPending(senddata_, senddatamutex_, ck, ol);
        }

        void PendingRecvSuccess(CompletionKey *ck, Overlapped *ol)
        {
            LockPending(recvdata_, recvdatamutex_, ck, ol);
        }

        void PendingAcceptSuccess(CompletionKey *ck, Overlapped *ol)
        {
            LockPending(acceptdata_, acceptdatamutex_, ck, ol);
        }

        void PendingConnectSuccess(CompletionKey *ck, Overlapped *ol)
        {
            LockPending(connectdata_, connectdatamutex_, ck, ol);
        }

        void PendingCloseSocket(CompletionKey *ck, Overlapped *ol)
        {
            LockPending(needclosesockets_, needclosesocketsmutex_, ck, ol);
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

        void GetAllNeedCloseSockets(PendingData& data)
        {
            LockSwapData(needclosesockets_, data, needclosesocketsmutex_);
        }

    private:
        template<typename DataType, typename MutexType>
        void LockSwapData(DataType& data1, DataType& data2, MutexType& mutex)
        {
            typename LockerType<MutexType>::type locker(mutex);
            data1.swap(data2);
        }

        template<typename MutexType>
        void LockPending(PendingData& data, MutexType& mutex,
                CompletionKey *ck, Overlapped *ol)
        {
            typename LockerType<MutexType>::type locker(mutex);
            data.push_back(std::make_pair(ck, ol));
        }

        // all pending data
        PendingData senddata_;
        PendingData recvdata_;
        PendingData acceptdata_;
        PendingData connectdata_;
        PendingData needclosesockets_;

        // all mutex
        SpinlocksMutex senddatamutex_;
        SpinlocksMutex recvdatamutex_;
        SpinlocksMutex acceptdatamutex_;
        SpinlocksMutex connectdatamutex_;
        SpinlocksMutex needclosesocketsmutex_;
    };

    class IocpService : private NotCopyable
    {
        struct ServiceThreadLocalData
        {
            HANDLE iocp;
            CompleteOperations& data;

            ServiceThreadLocalData(
                    HANDLE iocphandle,
                    CompleteOperations& co)
                : iocp(iocphandle),
                  data(co)
            {
            }
        };

    public:
        IocpService();
        ~IocpService();

        void RegisterRecvDataCallback(RecvDataCallback callback)
        {
            therecvcallback_ = callback;
        }

        template<typename SendHandler>
        void AsyncSend(SOCKET sock, const Buffer& buffer, SendHandler sendhandler)
        {
            Overlapped *ol = iocpdata_.NewOverlapped();
            ol->ot = SEND;
            ol->buf.buf = buffer.Get();
            ol->buf.len = buffer.Len();
            ol->buffer = buffer;
            ol->callback = sendhandler;
            ::WSASend(sock, &ol->buf, 1, 0, 0, (LPWSAOVERLAPPED)ol, 0);
            IncreaseOutStandingOl(sock);
        }

        template<typename RecvHandler>
        void AsyncRecv(SOCKET sock, Buffer& buffer, RecvHandler recvhandler)
        {
            DWORD flags = 0;
            Overlapped *ol = iocpdata_.NewOverlapped();
            ol->ot = RECV;
            ol->buf.buf = buffer.Get();
            ol->buf.len = buffer.Len();
            ol->buffer = buffer;
            ol->callback = recvhandler;
            ::WSARecv(sock, &ol->buf, 1, 0, &flags, (LPWSAOVERLAPPED)ol, 0);
            IncreaseOutStandingOl(sock);
        }

        template<typename ConnectHandler>
        void AsyncConn(SOCKET sock, const Address& address, const Port& port, ConnectHandler connhandler)
        {
            LPFN_CONNECTEX ConnectEx = 0;
            unsigned long retbytes = 0;
            GUID guid = WSAID_CONNECTEX;

            if (::WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
                        (LPVOID)&guid, sizeof(guid),
                        (LPVOID)&ConnectEx, sizeof(ConnectEx),
                        &retbytes, 0, 0))
                throw BaseException("can not get ConnectEx function at runtime!");

            Overlapped *ol = iocpdata_.NewOverlapped();
            ol->ot = CONNECT;
            ol->callback = connhandler;

            try {
                sockaddr_in local = Ipv4Address(Address(), Port(0));
                if (::bind(sock, (sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
                    throw BaseException("bind connect socket error!");

                sockaddr_in name = Ipv4Address(address, port);
                if (!ConnectEx(sock, (sockaddr *)&name, sizeof(name), 0, 0, 0, (LPOVERLAPPED)ol))
                {
                    if (::WSAGetLastError() != ERROR_IO_PENDING)
                        throw BaseException("call ConnectEx error!");
                }
                IncreaseOutStandingOl(sock);
                SetSocketAddr(sock, name);
            } catch (...) {
                iocpdata_.FreeOverlapped(ol);
                throw ;
            }
        }

        template<typename AcceptHandler>
        void AsyncAccept(SOCKET sock, AcceptHandler accepthandler)
        {
            LPFN_ACCEPTEX AcceptEx = 0;
            unsigned long retbytes = 0;
            GUID guid = WSAID_ACCEPTEX;

            if (::WSAIoctl(sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
                        (LPVOID)&guid, sizeof(guid),
                        (LPVOID)&AcceptEx, sizeof(AcceptEx),
                        &retbytes, 0, 0))
                throw BaseException("can not get AcceptEx function at runtime!");

            Overlapped *ol = iocpdata_.NewOverlapped();
            ol->ot = ACCEPT;
            ol->accepted = NewSocket();
            ol->callback = accepthandler;

            if (!AcceptEx(sock, ol->accepted, acceptaddrbuf_.GetAddrBuf(ol->accepted), 0,
                sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, 0, (LPOVERLAPPED)ol))
            {
                if (::WSAGetLastError() != ERROR_IO_PENDING)
                {
                    CloseSocket(ol->accepted);
                    iocpdata_.FreeOverlapped(ol);
                    throw BaseException("call AcceptEx error!");
                }
            }
            IncreaseOutStandingOl(sock);
        }

        SOCKET NewSocket();
        ISocketStream * GetIStream(SOCKET socket) const;
        OSocketStream * GetOStream(SOCKET socket) const;
        void CloseSocket(SOCKET socket);

        SOCKET NewAcceptor();
        void CloseAcceptor(SOCKET socket);

        void Run();

        Buffer CreateBuffer(std::size_t size)
        {
            return bufservice_.GetBuffer(size);
        }

        void DestroyBuffer(Buffer& buffer)
        {
            bufservice_.FreeBuffer(buffer);
        }

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

        typedef std::map<SOCKET, CompletionKey *> SocketHub;
        typedef std::map<SOCKET, ISocketStream *> ISocketStreamHub;
        typedef std::map<SOCKET, OSocketStream *> OSocketStreamHub;

        static std::size_t GetServiceThreadCount();
        static unsigned __stdcall ServiceThread(void *arg);

        void IncreaseOutStandingOl(SOCKET socket);
        void DecreaseOutStandingOl(CompletionKey *ck, Overlapped *ol);
        SOCKET CreateSocket();
        void DestroySocket(SOCKET socket);
        void DestroyAssociateStream(SOCKET socket);

        void IStreamRecv(SOCKET socket);
        void SetSocketAddr(SOCKET socket, const sockaddr_in& addr);

        void ProcessCompletedSend();
        void ProcessCompletedRecv();
        void ProcessCompletedAccept();
        void ProcessCompletedConnect();
        void ProcessNeedCloseSockets();

        IocpData iocpdata_;
        AcceptAddrBuf acceptaddrbuf_;
        RecvDataCallback therecvcallback_;

        SocketHub sockethub_;
        ISocketStreamHub istreamhub_;
        OSocketStreamHub ostreamhub_;
        DefaultBufferService bufservice_;

        CompleteOperations completeoperations_;
        CompleteOperations::PendingData sendcompletes_;
        CompleteOperations::PendingData recvcompletes_;
        CompleteOperations::PendingData acceptcompletes_;
        CompleteOperations::PendingData connectcompletes_;
        CompleteOperations::PendingData needclosesockets_;

        std::vector<Thread *> servicethreads_;
        HANDLE servicehandle_;
    };
} // namespace bittorrent

#endif // _IOCP_SERVICE_H_

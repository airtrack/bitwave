#include "IocpService.h"

#include <assert.h>
#include "../Socket.h"
#include "../ISocketStream.h"
#include "../OSocketStream.h"

namespace bittorrent
{
    IocpService::IocpService()
        : iocpdata_(),
          acceptaddrbuf_(),
          sockethub_(),
          istreamhub_(),
          ostreamhub_(),
          bufservice_(),
          completeoperations_(),
          servicehandle_(INVALID_HANDLE_VALUE)
    {
        servicehandle_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
        if (!servicehandle_) throw BaseException("can not create iocp service!");

        std::size_t threadcount = GetServiceThreadCount();
        for (std::size_t i = 0; i < threadcount; ++i)
            Thread(ServiceThread, new ServiceThreadLocalData(servicehandle_, completeoperations_));
    }

    SOCKET IocpService::NewSocket()
    {
        SOCKET sock = CreateSocket();
        istreamhub_.insert(std::make_pair(sock, new ISocketStream()));
        ostreamhub_.insert(std::make_pair(sock, new OSocketStream()));
    }

    ISocketStream * IocpService::GetIStream(SOCKET socket) const
    {
        ISocketStreamHub::const_iterator it = istreamhub_.find(socket);
        if (it == istreamhub_.end())
            return 0;
        return it->second;
    }

    OSocketStream * IocpService::GetOStream(SOCKET socket) const
    {
        OSocketStreamHub::const_iterator it = ostreamhub_.find(socket);
        if (it == ostreamhub_.end())
            return 0;
        return it->second;
    }

    void IocpService::CloseSocket(SOCKET socket)
    {
        DestroyAssociateStream(socket);
        DestroySocket(socket);
    }

    SOCKET IocpService::NewAcceptor()
    {
        return CreateSocket();
    }

    void IocpService::CloseAcceptor(SOCKET socket)
    {
        DestroySocket(socket);
    }

    void IocpService::Run()
    {
        ProcessCompletedSend();
        ProcessCompletedRecv();
        ProcessCompletedAccept();
        ProcessCompletedConnect();
        ProcessNeedCloseSockets();
    }

    // static
    std::size_t IocpService::GetServiceThreadCount()
    {
        SYSTEM_INFO sysinfo;
        ::GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors * 2 + 2;
    }

    // static
    unsigned __stdcall IocpService::ServiceThread(void *arg)
    {
        ScopePtr<ServiceThreadLocalData> ptr(
                static_cast<ServiceThreadLocalData *>(arg));
        unsigned long numofbytes;
        CompletionKey *ck = 0;
        Overlapped *ol = 0;

        while (true)
        {
            if (::GetQueuedCompletionStatus(ptr->iocp, &numofbytes,
                        (PULONG_PTR)&ck, (LPOVERLAPPED *)&ol, INFINITE))
            {
                if (numofbytes == 0 && (ol->ot == SEND || ol->ot == RECV))
                {
                    ptr->data.PendingCloseSocket(ck, ol);
                    continue;
                }

                ol->bufused = numofbytes;
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
                    ptr->data.PendingCloseSocket(ck, ol);
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

    void IocpService::IncreaseOutStandingOl(SOCKET socket)
    {
        SocketHub::iterator it = sockethub_.find(socket);
        if (it == sockethub_.end()) return ;
        it->second->outstandingols++;
    }

    void IocpService::DecreaseOutStandingOl(CompletionKey *ck, Overlapped *ol)
    {
        if (ol->buffer) DestroyBuffer(ol->buffer);
        iocpdata_.FreeOverlapped(ol);
        ck->outstandingols--;
    }

    SOCKET IocpService::CreateSocket()
    {
        SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET)
            throw BaseException("create a new socket error!");

        // add to iocp
        CompletionKey *ck = iocpdata_.NewCompletionKey();
        if (!::CreateIoCompletionPort((HANDLE)sock, servicehandle_, (ULONG_PTR)ck, 0))
        {
            iocpdata_.FreeCompletionKey(ck);
            ::closesocket(sock);
            throw BaseException("add socket to iocp error!");
        }

        sockethub_.insert(std::make_pair(sock, ck));
        return sock;
    }

    void IocpService::DestroySocket(SOCKET socket)
    {
        SocketHub::iterator it = sockethub_.find(socket);
        if (it == sockethub_.end())
            throw BaseException("the socket is not in this io service!");

        if (it->second->outstandingols == 0)
        {
            // free CompletionKey
            iocpdata_.FreeCompletionKey(it->second);
        }
        else
        {
            // mark as INVALID_SOCKET
            it->second->sock = INVALID_SOCKET;
        }
        sockethub_.erase(it);
        ::closesocket(socket);
    }

    void IocpService::DestroyAssociateStream(SOCKET socket)
    {
        ISocketStreamHub::iterator it1 = istreamhub_.find(socket);
        OSocketStreamHub::iterator it2 = ostreamhub_.find(socket);
        if (it1 != istreamhub_.end())
        {
            delete it1->second;
            istreamhub_.erase(it1);
        }

        if (it2 != ostreamhub_.end())
        {
            delete it2->second;
            ostreamhub_.erase(it2);
        }
    }

    void IocpService::ProcessCompletedSend()
    {
        completeoperations_.GetAllSendSuccess(sendcompletes_);
        for (auto it = sendcompletes_.begin(); it != sendcompletes_.end(); ++it)
        {
            // for send, we may let callback empty
            if (it->second->callback)
            {
                SocketHandler sock(*this, it->first->sock,
                        GetIStream(it->first->sock),
                        GetOStream(it->first->sock));
                it->second->callback(sock, it->second->buffer);
            }
            DecreaseOutStandingOl(it->first, it->second);
        }
        sendcompletes_.clear();
    }

    void IocpService::ProcessCompletedRecv()
    {
        completeoperations_.GetAllRecvSuccess(recvcompletes_);
        for (auto it = recvcompletes_.begin(); it != recvcompletes_.end(); ++it)
        {
            SocketHandler sock(*this, it->first->sock,
                    GetIStream(it->first->sock),
                    GetOStream(it->first->sock));
            assert(it->second->callback);
            it->second->callback(sock,
                it->second->buffer, it->second->bufused);
            DecreaseOutStandingOl(it->first, it->second);
        }
        recvcompletes_.clear();
    }

    void IocpService::ProcessCompletedAccept()
    {
        completeoperations_.GetAllAcceptSuccess(acceptcompletes_);
        for (auto it = acceptcompletes_.begin(); it != acceptcompletes_.end(); ++it)
        {
            AcceptorHandler acceptor(*this, it->first->sock);
            SocketHandler sock(*this, it->second->accepted,
                    GetIStream(it->second->accepted),
                    GetOStream(it->second->accepted));
            assert(it->second->callback);
            it->second->callback(acceptor, sock);
            DecreaseOutStandingOl(it->first, it->second);
        }
        acceptcompletes_.clear();
    }

    void IocpService::ProcessCompletedConnect()
    {
        completeoperations_.GetAllConnectSuccess(connectcompletes_);
        for (auto it = connectcompletes_.begin(); it != connectcompletes_.end(); ++it)
        {
            SocketHandler sock(*this, it->first->sock,
                    GetIStream(it->first->sock),
                    GetOStream(it->first->sock));
            assert(it->second->callback);
            it->second->callback(sock);
            DecreaseOutStandingOl(it->first, it->second);
        }
        connectcompletes_.clear();
    }

    void IocpService::ProcessNeedCloseSockets()
    {
        completeoperations_.GetAllNeedCloseSockets(needclosesockets_);
        for (auto it = needclosesockets_.begin(); it != needclosesockets_.end(); ++it)
        {
            DecreaseOutStandingOl(it->first, it->second);
            if (it->first->sock != INVALID_SOCKET)
            {
                DestroyAssociateStream(it->first->sock);
                sockethub_.erase(it->first->sock);
                ::closesocket(it->first->sock);
                it->first->sock = INVALID_SOCKET;
            }
            if (it->first->outstandingols == 0)
            {
                iocpdata_.FreeCompletionKey(it->first);
            }
        }
        needclosesockets_.clear();
    }

    // IocpService::AcceptAddrBuf -----------------------------------------------

    char * IocpService::AcceptAddrBuf::GetAddrBuf(SOCKET sock)
    {
        SocketAddrBuf::iterator it = socketaddrbuf_.find(sock);
        if (it == socketaddrbuf_.end())
        {
            Buffer buf = addrbuf_.GetBuffer(addrbufsize);
            it = socketaddrbuf_.insert(socketaddrbuf_.begin(), std::make_pair(sock, buf.Get()));
        }

        return it->second;
    }

    void IocpService::AcceptAddrBuf::ReleaseAddrBuf(SOCKET sock)
    {
        SocketAddrBuf::iterator it = socketaddrbuf_.find(sock);
        if (it == socketaddrbuf_.end())
            return ;

        char *buf = it->second;
        socketaddrbuf_.erase(it);
        addrbuf_.FreeBuffer(buf, addrbufsize);
    }
} // namespace bittorrent

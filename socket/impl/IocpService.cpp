#include "IocpService.h"
#include "Socket.h"

namespace bittorrent
{

    IocpService::IocpService()
        : iocpdata_(),
        socketmanager_(),
        iosocketedmanager_(iocpdata_),
        acceptaddrbuf_(),
        completeoperations_(),
        servicehandle_(INVALID_HANDLE_VALUE)
    {
        servicehandle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
        if (!servicehandle_) throw BaseException("can not create iocp service!");

        std::size_t threadcount = GetServiceThreadCount();
        for (std::size_t i = 0; i < threadcount; ++i)
            Thread(ServiceThread, new ServiceThreadLocalData(servicehandle_, completeoperations_));
    }

    void IocpService::Run()
    {
        ProcessCompletedSend();
        ProcessCompletedRecv();
        ProcessCompletedAccept();
        ProcessCompletedConnect();
        ProcessNeedCloseSockets();
    }

    SOCKET IocpService::GetSocket()
    {
        SOCKET sock = socketmanager_.NewSocket();
        CompletionKey *ck = iocpdata_.NewCompletionKey();
        ck->sock = sock;
        CreateIoCompletionPort((HANDLE)sock, servicehandle_, (ULONG_PTR)ck, 0);
        iosocketedmanager_.AddSocket(sock, ck);
        return sock;
    }

    void IocpService::FreeSocket(SOCKET sock)
    {
        iosocketedmanager_.FreeSocket(sock);
        socketmanager_.FreeSocket(sock);
    }

    // static
    std::size_t IocpService::GetServiceThreadCount()
    {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors * 2 + 2;
    }

    // static
    unsigned __stdcall IocpService::ServiceThread(void *arg)
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
                if (numofbytes == 0 && (ol->ot == SEND || ol->ot == RECV))
                {
                    ptr->data.PendingCloseSocket(ck->sock);
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

    void IocpService::ProcessCompletedSend()
    {
        completeoperations_.GetAllSendSuccess(sendcompletes_);
        for (auto it = sendcompletes_.begin(); it != sendcompletes_.end(); ++it)
        {
            SocketHandler sock(it->first->sock, *this);
            it->second->callback(sock,
                it->second->buf.buf, it->second->buf.len);
            iosocketedmanager_.UnBindSocket(it->first->sock, it->second);
        }
        sendcompletes_.clear();
    }

    void IocpService::ProcessCompletedRecv()
    {
        completeoperations_.GetAllRecvSuccess(recvcompletes_);
        for (auto it = recvcompletes_.begin(); it != recvcompletes_.end(); ++it)
        {
            SocketHandler sock(it->first->sock, *this);
            it->second->callback(sock,
                it->second->buf.buf, it->second->buf.len, it->second->bufused);
            iosocketedmanager_.UnBindSocket(it->first->sock, it->second);
        }
        recvcompletes_.clear();
    }

    void IocpService::ProcessCompletedAccept()
    {
        completeoperations_.GetAllAcceptSuccess(acceptcompletes_);
        for (auto it = acceptcompletes_.begin(); it != acceptcompletes_.end(); ++it)
        {
            AcceptorHandler acceptor(it->first->sock, *this);
            SocketHandler sock(it->second->accepted, *this);
            it->second->callback(acceptor, sock);
            iosocketedmanager_.UnBindSocket(it->first->sock, it->second);
        }
        acceptcompletes_.clear();
    }

    void IocpService::ProcessCompletedConnect()
    {
        completeoperations_.GetAllConnectSuccess(connectcompletes_);
        for (auto it = connectcompletes_.begin(); it != connectcompletes_.end(); ++it)
        {
            SocketHandler sock(it->first->sock, *this);
            it->second->callback(sock);
            iosocketedmanager_.UnBindSocket(it->first->sock, it->second);
        }
        connectcompletes_.clear();
    }

    void IocpService::ProcessNeedCloseSockets()
    {
        completeoperations_.GetAllNeedCloseSockets(needclosesockets_);
        for (auto it = needclosesockets_.begin(); it != needclosesockets_.end(); ++it)
            FreeSocket(*it);
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

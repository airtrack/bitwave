#include "IoService.h"
#include "Socket.h"

namespace bittorrent
{

    void IoService::ProcessCompletedSend()
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

    void IoService::ProcessCompletedRecv()
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

    void IoService::ProcessCompletedAccept()
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

    void IoService::ProcessCompletedConnect()
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

    void IoService::ProcessNeedCloseSockets()
    {
        completeoperations_.GetAllNeedCloseSockets(needclosesockets_);
        for (auto it = needclosesockets_.begin(); it != needclosesockets_.end(); ++it)
            FreeSocket(*it);
        needclosesockets_.clear();
    }

} // namespace bittorrent

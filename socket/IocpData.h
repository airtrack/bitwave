#ifndef _IOCP_DATA_H_
#define _IOCP_DATA_H_

#include <WinSock2.h>
#include "functor.h"

namespace bittorrent
{
    enum OperateType
    {
        ACCEPT,
        CONNECT,
        RECV,
        SEND,
    };

    struct CompletionKey
    {
        SOCKET sock;
        sockaddr_in addr;
    };

    struct Overlapped
    {
        OVERLAPPED ol;
        OperateType ot;
        WSABUF buf;
        SOCKET accepted;
        Functor callback;
    };

    class IocpData
    {
    public:
        IocpData();

        CompletionKey * NewCompletionKey();
        void FreeCompletionKey(CompletionKey *ck);

        Overlapped * NewOverlapped();
        void FreeOverlapped(Overlapped *ol);
    };
} // namespace bittorrent

#endif // _IOCP_DATA_H_

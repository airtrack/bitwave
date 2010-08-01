#ifndef _IOCP_DATA_H_
#define _IOCP_DATA_H_

#include <WinSock2.h>
#include "IoServiceCallback.h"
#include "../base/ObjectPool.h"

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

        static void Init(CompletionKey *ck, std::size_t size = 1)
        {
            for (std::size_t i = 0; i < size; ++i)
            {
                ck[i].sock = INVALID_SOCKET;
                memset(&ck[i].addr, 0, sizeof(sockaddr_in));
            }
        }
    };

    struct Overlapped
    {
        OVERLAPPED ol;
        OperateType ot;
        WSABUF buf;
        SOCKET accepted;
        IoServiceCallback callback;

        static void Init(Overlapped *ol, std::size_t size = 1)
        {
            for (std::size_t i = 0; i < size; ++i)
            {
                memset(&ol[i].ol, 0, sizeof(OVERLAPPED));
                ol[i].ot = ACCEPT;
                ol[i].buf.len = 0;
                ol[i].buf.buf = 0;
                ol[i].accepted = INVALID_SOCKET;
                ol[i].callback.Release();
            }
        }
    };

    class IocpData
    {
    public:
        IocpData()
            : ckpool_(),
              olpool_()
        {
        }

        CompletionKey * NewCompletionKey()
        {
            return ckpool_.ObtainObject();
        }

        void FreeCompletionKey(CompletionKey *ck)
        {
            ckpool_.ReturnObject(ck);
        }

        Overlapped * NewOverlapped()
        {
            return olpool_.ObtainObject();
        }

        void FreeOverlapped(Overlapped *ol)
        {
            olpool_.ReturnObject(ol);
        }

    private:
        ObjectPool<CompletionKey, CompletionKey> ckpool_;
        ObjectPool<Overlapped, Overlapped> olpool_;
    };
} // namespace bittorrent

#endif // _IOCP_DATA_H_

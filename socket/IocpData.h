#ifndef _IOCP_DATA_H_
#define _IOCP_DATA_H_

#include <WinSock2.h>
#include "../base/ObjectPool.h"
#include "Buffer.h"

namespace bittorrent
{
    namespace socket
    {
        struct CompletionKey
        {
            SOCKET sock;
            sockaddr_in addr;
        };

        struct OverLapped
        {
            OVERLAPPED ol;
            WSABUF wsabuf;
            Buffer buf;
            int optype;
        };

        class IocpData
        {
        public:
            IocpData();

            CompletionKey * ObtainCompletionKey();
            void ReturnCompletionKey(CompletionKey *ck);
            OverLapped * ObtainOverLapped();
            void ReturnOverLapped(OverLapped *ol);

        private:
            ObjectPool<CompletionKey> completionkeypool_;
            ObjectPool<OverLapped> overlappedpool_;
        };
    } // namespace socket
} // namespace bittorrent

#endif // _IOCP_DATA_H_
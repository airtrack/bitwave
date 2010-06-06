#include "IoService.h"
#include "Buffer.h"
#include "Socket.h"

namespace bittorrent
{
    namespace socket
    {
        IoService::IoService()
            : handle_(0)
        {
            handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
            if (!handle_)
                throw IoException("can not init io service!");
        }

        void IoService::Send(Socket *socket, Buffer& buf)
        {
        }

        void IoService::Recv(Socket *socket, Buffer& buf)
        {
        }

        void IoService::Connect(Socket *socket, const sockaddr *name, int namelen)
        {
        }

        void IoService::Accept(Acceptor *acceptor, Socket& socket)
        {
        }
    } // namespace bittorrent
} // namespace bittorrent
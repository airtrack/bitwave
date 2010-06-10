#include "IoService.h"
#include "Buffer.h"
#include "Socket.h"
#include <MSWSock.h>

namespace bittorrent
{
    namespace socket
    {
        IoService::IoService()
            : handle_(0),
              iocpdata_(),
              flag_(0)
        {
            handle_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
            if (!handle_)
                throw IoException("can not init io service!");
        }

        void IoService::AddClient(SOCKET sock, const sockaddr_in& addr)
        {
            CompletionKey *ck = iocpdata_.ObtainCompletionKey();
            ck->sock = sock;
            ck->addr = addr;
            CreateIoCompletionPort((HANDLE)sock, handle_, (ULONG_PTR)ck, 0);
        }

        void IoService::Send(Socket *socket, Buffer& buf)
        {
            OverLapped *ol = iocpdata_.ObtainOverLapped();
            ol->buf = buf;
            ol->wsabuf.len = ol->buf.used;
            ol->wsabuf.buf = ol->buf.buf;
            ol->optype = SEND;
            memset(&buf, 0, sizeof(buf));
            WSASend(socket->GetRawSock(), &ol->wsabuf, 1, 0, 0, (LPWSAOVERLAPPED)ol, 0);
        }

        void IoService::Recv(Socket *socket, Buffer& buf)
        {
            OverLapped *ol = iocpdata_.ObtainOverLapped();
            ol->buf = buf;
            ol->wsabuf.len = ol->buf.buflen;
            ol->wsabuf.buf = ol->buf.buf;
            ol->optype = RECEIVE;
            memset(&buf, 0, sizeof(buf));
            WSARecv(socket->GetRawSock(), &ol->wsabuf, 1, 0, &flag_, (LPWSAOVERLAPPED)ol, 0);
        }

        void IoService::Connect(Socket *socket, const sockaddr *name, int namelen)
        {
            LPFN_CONNECTEX ConnectEx = 0;
            unsigned long retbytes = 0;
            GUID guid = WSAID_CONNECTEX;

            if (WSAIoctl(socket->GetRawSock(),
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                (LPVOID)&guid, sizeof(guid),
                (LPVOID)&ConnectEx, sizeof(ConnectEx),
                &retbytes, 0, 0))
                throw IoException("can not get ConnectEx function pointer!");

            OverLapped *ol = iocpdata_.ObtainOverLapped();
            ol->optype = CONNECT;

            ConnectEx(socket->GetRawSock(), name, namelen, 0, 0, 0, (LPOVERLAPPED)ol);
        }

        void IoService::Accept(Acceptor *acceptor, Socket& socket)
        {
            LPFN_ACCEPTEX AcceptEx = 0;
            unsigned long retbytes = 0;
            GUID guid = WSAID_ACCEPTEX;

            if (WSAIoctl(acceptor->GetRawSock(),
                SIO_GET_EXTENSION_FUNCTION_POINTER,
                (LPVOID)&guid, sizeof(guid),
                (LPVOID)&AcceptEx, sizeof(AcceptEx),
                &retbytes, 0, 0))
                throw IoException("can not get AcceptEx function pointer!");

            OverLapped *ol = iocpdata_.ObtainOverLapped();
            ol->optype = ACCEPT;

            AcceptEx(acceptor->GetRawSock(), socket.GetRawSock(), 0, 0, 0, 0, 0, (LPOVERLAPPED)ol);
        }
    } // namespace bittorrent
} // namespace bittorrent
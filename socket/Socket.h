#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <WinSock2.h>
#include <cstddef>
#include "../base/BaseTypes.h"

namespace bittorrent
{
    namespace socket
    {
        class SocketException : BaseException
        {
        public:
            SocketException(const char *w)
                : BaseException(w)
            {
            }
        };

        struct Buffer
        {
            Buffer(char *b, std::size_t bl)
                : buf(b), buflen(bl), used(0) { }

            char *buf;
            std::size_t buflen;
            std::size_t used;
        };

        class BufferAllocator
        {
        public:
            static Buffer AllocBuf(std::size_t size);
            static void DeallocBuf(Buffer& buf);
        };

        class Address
        {
        public:
            static const unsigned long any = INADDR_ANY;

            Address();
            Address(unsigned long hladdress);
            Address(const char *address);

            operator unsigned long () const { return address_; }

        private:
            unsigned long address_;
        };

        class Port
        {
        public:
            Port(unsigned short hsport);
            operator unsigned short () const { return port_; }

        private:
            unsigned short port_;
        };

        class IoService;

        class Socket
        {
        public:
            Socket();
            explicit Socket(IoService *service);
            ~Socket();

            void Connect(const Address& address, const Port& port);
            void Send(Buffer& buf);
            void Recv(Buffer& buf);
            void Close();
            SOCKET GetRawSock() const;

        private:
            void CheckServiceValid() const;
            SOCKET sock_;
            IoService *service_;
        };

        class Acceptor
        {
        public:
            Acceptor(IoService *service, const Address& address, const Port& port);

            void Accept(Socket& sock);
            void Close();
            SOCKET GetRawSock() const;

        private:
            void CheckServiceValid() const;
            SOCKET sock_;
            IoService *service_;
        };

        class IoService
        {
        public:
            IoService();

            void Send(Socket *socket, Buffer& buf);
            void Recv(Socket *socket, Buffer& buf);
            void Connect(Socket *socket, const sockaddr *name, int namelen);
            void Accept(Acceptor *acceptor, Socket& socket);

        private:
            HANDLE handle_;
        };
    } // namespace socket
} // namespace bittorrent

#endif // _SOCKET_H_
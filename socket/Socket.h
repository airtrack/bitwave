#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <WinSock2.h>
#include <cstddef>

namespace bittorrent
{
    namespace socket
    {
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
            static const long any = INADDR_ANY;

            Address();
            Address(long hladdress);
            Address(const char *address);

            operator long () const { return address_; }

        private:
            long address_;
        };

        class Port
        {
        public:
            Port(short hsport);
            operator short () const { return port_; }

        private:
            short port_;
        };

        class IoService;

        class Socket
        {
        public:
            Socket();

            void Connect(const Address& address, const Port& port);
            void Send(Buffer& buf);
            void Recv(Buffer& buf);
            void Close();
            SOCKET GetRawSock() const;

        private:
            SOCKET sock_;
            IoService *service_;
        };

        class Acceptor
        {
        public:
            Acceptor(const Address& address, const Port& port);

            void Accept(Socket& sock);
            void Close();
            SOCKET GetRawSock() const;

        private:
            SOCKET sock_;
            IoService *service_;
        };

        class IoService
        {
        public:
            typedef HANDLE ServiceHandle;
            IoService();

            void Send(Socket *socket, Buffer& buf);
            void Recv(Socket *socket, Buffer& buf);
            void Connect(Socket *socket, const sockaddr *name);
            void Accept(Acceptor *acceptor, Socket& socket);

        private:
            ServiceHandle handle_;
        };
    } // namespace socket
} // namespace bittorrent

#endif // _SOCKET_H_
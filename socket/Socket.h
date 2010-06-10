#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <WinSock2.h>
#include "../base/BaseTypes.h"

namespace bittorrent
{
    namespace socket
    {
        class SocketException : public BaseException
        {
        public:
            SocketException(const char *w)
                : BaseException(w)
            {
            }
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
        class Buffer;

        class Socket
        {
        public:
            explicit Socket(IoService *service);

            void Connect(const Address& address, const Port& port);
            void Send(Buffer& buf);
            void Recv(Buffer& buf);
            void Close();
            SOCKET GetRawSock() const;

        private:
            void CheckValid() const;

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
            SOCKET sock_;
            IoService *service_;
        };
    } // namespace socket
} // namespace bittorrent

#endif // _SOCKET_H_
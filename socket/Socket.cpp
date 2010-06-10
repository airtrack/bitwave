#include "Socket.h"
#include "IoService.h"

namespace bittorrent
{
    namespace socket
    {
        Address::Address()
            : address_(htonl(any))
        {
        }

        Address::Address(unsigned long hladdress)
            : address_(htonl(hladdress))
        {
        }

        Address::Address(const char *address)
            : address_(inet_addr(address))
        {
        }

        Port::Port(unsigned short hsport)
            : port_(htons(hsport))
        {
        }

        Socket::Socket(IoService *service)
            : sock_(INVALID_SOCKET),
              service_(service)
        {
            if (service_ == 0)
                throw SocketException("it is no valid service in socket!");

            sock_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock_ == INVALID_SOCKET)
                throw SocketException("can not create a socket!");
        }

        void Socket::Connect(const Address& address, const Port& port)
        {
            CheckValid();
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = port;
            addr.sin_addr.s_addr = address;
            BindService(sock_, addr, service_);
            service_->Connect(this, (sockaddr *)&addr, sizeof(addr));
        }

        void Socket::Send(Buffer& buf)
        {
            CheckValid();
            service_->Send(this, buf);
        }

        void Socket::Recv(Buffer& buf)
        {
            CheckValid();
            service_->Recv(this, buf);
        }

        void Socket::Close()
        {
            if (sock_ != INVALID_SOCKET)
            {
                closesocket(sock_);
                sock_ = INVALID_SOCKET;
                service_ = 0;
            }
        }

        SOCKET Socket::GetRawSock() const
        {
            return sock_;
        }

        void Socket::CheckValid() const
        {
            if (sock_ == INVALID_SOCKET)
                throw SocketException("socket is invalid in Socket class!");
        }

        Acceptor::Acceptor(IoService *service, const Address& address, const Port& port)
            : sock_(INVALID_SOCKET),
              service_(service)
        {
            if (service_ == 0)
                throw SocketException("it is no valid service in acceptor!");

            sock_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock_ == INVALID_SOCKET)
                throw SocketException("can not create socket in acceptor!");

            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = port;
            addr.sin_addr.s_addr = address;
            if (::bind(sock_, (sockaddr *)&addr, sizeof(addr))
                || ::listen(sock_, SOMAXCONN))
            {
                Close();
                throw SocketException("can not listen port in acceptor!");
            }

            BindService(sock_, addr, service_);
        }

        void Acceptor::Accept(Socket& sock)
        {
            if (sock_ == INVALID_SOCKET)
                throw SocketException("there is no valid socket in acceptor!");

            service_->Accept(this, sock);
        }

        void Acceptor::Close()
        {
            if (sock_ != INVALID_SOCKET)
            {
                closesocket(sock_);
                sock_ = INVALID_SOCKET;
                service_ = 0;
            }
        }

        SOCKET Acceptor::GetRawSock() const
        {
            return sock_;
        }
    } // namespace socket
} // namespace bittorrent
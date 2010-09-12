#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "Address.h"
#include "IoService.h"
#include "ISocketStream.h"
#include "OSocketStream.h"

namespace bittorrent
{
    class SocketHandler
    {
        friend class ISocketStream;
    public:
        // create new socket
        explicit SocketHandler(IoService& service)
            : service_(service),
              socket_(INVALID_SOCKET),
              istream_(0),
              ostream_(0)
        {
            socket_ = service_.NewSocket();
            istream_ = service_.GetIStream(socket_);
            ostream_ = service_.GetOStream(socket_);
        }

        // create a socket handler of exist socket
        SocketHandler(IoService& service, SOCKET socket,
                ISocketStream *istream, OSocketStream *ostream)
            : service_(service),
              socket_(socket),
              istream_(istream),
              ostream_(ostream_)
        {
        }

        template<typename ConnectHandler>
        void AsyncConnect(const Address& address, const Port& port,
                ConnectHandler handler)
        {
            service_.AsyncConn(socket_, address, port, handler);
        }

        void Close()
        {
            service_.CloseSocket(socket_);
        }

        SOCKET GetSocket() const
        {
            return socket_;
        }

        // Send data, return immediately
        void Send(const char *data, std::size_t size)
        {
            ostream_->Write(data, size);
        }

        // Send data, return immediately, the 'buffer' need call IoService's
        // CreateBuffer() to obtain
        void Send(Buffer& buffer)
        {
            ostream_->Write(buffer);
        }

        int Peek() const
        {
            return istream_->Peek();
        }

        std::size_t Peekn(char *peeked, std::size_t n)
        {
            return istream_->Peekn(peeked, n);
        }

        // Recv data
        std::size_t Recv(char *buf, std::size_t size)
        {
            return istream_->Read(buf, size);
        }

        // return size can be Recv
        std::size_t Reserved() const
        {
            return istream_->Reserved();
        }

        // return true Recv data is EOF
        bool eof() const
        {
            return istream_->eof();
        }

    private:
        IoService &service_;
        SOCKET socket_;
        ISocketStream *istream_;
        OSocketStream *ostream_;
    };

    class AcceptorHandler
    {
    public:
        // Create a new Acceptor from IoService
        AcceptorHandler(IoService& service, Port port, Address address = Address())
            : service_(service),
              socket_(service.NewAcceptor())
        {
            sockaddr_in listenaddr;
            listenaddr.sin_family = AF_INET;
            listenaddr.sin_port = port;
            listenaddr.sin_addr.s_addr = address;

            try
            {
                if (::bind(sock_, (sockaddr *)&listenaddr, sizeof(listenaddr)))
                    throw "can not bind acceptor!";
                if (::listen(sock_, SOMAXCONN))
                    throw "acceptor listen error!";
            } catch (...) {
                Close();
                throw;
            }
        }

        // Get an exist AcceptorHandler from IoService
        AcceptorHandler(IoService& service, SOCKET socket)
            : service_(service),
              socket_(socket)
        {
        }

        template<typename AcceptHandler>
        void AsyncAccept(AcceptHandler accepthandler)
        {
            service_.AsyncAccept(socket_, accepthandler);
        }

        SOCKET GetSocket() const
        {
            return socket_;
        }

        void Close()
        {
            service_.CloseAcceptor(socket_);
        }

    private:
        IoService& service_;
        SOCKET socket_;
    };
} // namespace bittorrent

#endif // _SOCKET_H_

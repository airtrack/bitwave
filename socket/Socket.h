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
    public:
        // Create new socket from IoService
        explicit SocketHandler(IoService& service)
            : service_(service),
              socket_(service.NewSocket()),
              istream_(service.GetIStream(socket_)),
              ostream_(service.GetOStream(socket_))
        {
        }

        // Get an exist socket from IoService
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
            service_.AsyncConnect(this, address, port, handler);
        }

        void Close()
        {
            service_.CloseSocket(this);
        }

        SOCKET GetSocket() const
        {
            return socket_;
        }

        // Send data
        Socket& Send(char *data, std::size_t size)
        {
            ostream_->Write(data, size);
            return *this;
        }

        // Flush Send data immediately
        Socket& Flush()
        {
            ostream_->Flush();
            return *this;
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
                if (bind(sock_, (sockaddr *)&listenaddr, sizeof(listenaddr)))
                    throw "can not bind acceptor!";
                if (listen(sock_, SOMAXCONN))
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
            service_.AsyncAccept(this, accepthandler);
        }

        SOCKET GetSocket() const
        {
            return socket_;
        }

        void Close()
        {
            service_.CloseAcceptor(this);
        }

    private:
        IoService& service_;
        SOCKET socket_;
    };
} // namespace bittorrent

#endif // _SOCKET_H_

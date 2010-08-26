#ifndef _IO_SERVICE_H_
#define _IO_SERVICE_H_

#include "../base/BaseTypes.h"
#include "impl/IocpService.h"

namespace bittorrent
{
    class SocketHandler;
    class AcceptorHandler;

    // this template class just a IoService interface, all real IoService
    // will implement all interface method do real work
    template<typename ServiceImpl>
    class IoServiceHolder
        : private NotCopyable
    {
    public:
        typedef ServiceImpl service_impl;

        SOCKET NewSocket()
        {
            return impl_.NewSocket();
        }

        ISocketStream * GetIStream(SOCKET socket)
        {
            return impl_.GetIStream(socket);
        }

        OSocketStream * GetOStream(SOCKET socket)
        {
            return impl_.GetOStream(socket);
        }

        template<typename ConnectHandler>
        void AsyncConnect(SocketHandler *sockethandler,
                const Address& address, const Port& port,
                ConnectHandler handler)
        {
            impl_.AsyncConnect(sockethandler, address, port, handler);
        }

        void CloseSocket(SocketHandler *handler)
        {
            impl_.CloseSocket(handler);
        }

        SOCKET NewAcceptor()
        {
            return impl_.NewAcceptor();
        }

        template<typename AcceptHandler>
        void AsyncAccept(AcceptorHandler *acceptor, AcceptHandler handler)
        {
            impl_.AsyncAccept(acceptor, handler);
        }

        void CloseAcceptor(AcceptorHandler *handler)
        {
            impl_.CloseAcceptor(handler);
        }

        void Run()
        {
            impl_.Run();
        }

    private:
        service_impl impl_;
    };

    typedef IoServiceHolder<IocpService> IoService;
} // namespace bittorrent

#endif // _IO_SERVICE_H_

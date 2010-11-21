#ifndef IOCP_H
#define IOCP_H

#include "../base/BaseTypes.h"
#include "Address.h"
#include "Overlapped.h"
#include <WinSock2.h>
#include <MSWSock.h>

namespace bittorrent {
namespace net {

    // iocp service exception error code
    enum
    {
        GET_ACCEPTEX_FUNCTION_ERROR,
        GET_CONNECTEX_FUNCTION_ERROR,
        CALL_ACCEPTEX_FUNCTION_ERROR,
        CALL_CONNECTEX_FUNCTION_ERROR,
        CALL_WSARECV_FUNCTION_ERROR,
        CALL_WSASEND_FUNCTION_ERROR,
        CONNECT_BIND_LOCAL_ERROR,
    };

    // an exception class for iocp service
    class IocpException
    {
    public:
        explicit IocpException(int code)
            : code_(code)
        {
        }

        int GetCode() const
        {
            return code_;
        }

    private:
        int code_;
    };

    // a class supply iocp service for sockets, sockets could use the
    // AsyncAccept, AsyncConnect, AsyncReceive, AsyncSend methods to
    // communicate with other socket
    class IocpService : private NotCopyable
    {
    public:
        void Breath();
        void RegisterSocket(SOCKET socket);

        template<typename ImplementType, typename Handler>
        void AsyncAccept(ImplementType impl, Handler handler)
        {
            unsigned long bytes = 0;
            LPFN_ACCEPTEX AcceptEx = 0;
            GUID guid = WSAID_ACCEPTEX;

            int error = ::WSAIoctl(impl.Get(), SIO_GET_EXTENSION_FUNCTION_POINTER,
                                   &guid, sizeof(guid), &AcceptEx, sizeof(AcceptEx),
                                   &bytes, 0, 0);
            if (error)
                throw IocpException(GET_ACCEPTEX_FUNCTION_ERROR);

            OverlappedPtr<AcceptOverlapped> ptr(new AcceptOverlapped(handler, *this));

            const int address_length = ptr->GetAddressLength();
            error = AcceptEx(impl.Get(), ptr->GetAcceptSocket(), ptr->GetAddressBuf(),
                             0, address_length, address_length, 0, ptr.Get());

            if (!error && ::WSAGetLastError() != ERROR_IO_PENDING)
                throw IocpException(CALL_ACCEPTEX_FUNCTION_ERROR);

            ptr.Release();
        }

        template<typename ImplementType, typename Handler>
        void AsyncConnect(ImplementType impl, const Address& address, const Port& port, Handler handler)
        {
            unsigned long bytes = 0;
            LPFN_CONNECTEX ConnectEx = 0;
            GUID guid = WSAID_CONNECTEX;

            int error = ::WSAIoctl(impl.Get(), SIO_GET_EXTENSION_FUNCTION_POINTER,
                                   &guid, sizeof(guid), &ConnectEx, sizeof(ConnectEx),
                                   &bytes, 0, 0);
            if (error)
                throw IocpException(GET_CONNECTEX_FUNCTION_ERROR);

            OverlappedPtr<ConnectOverlapped> ptr(new ConnectOverlapped(handler));

            sockaddr_in local = Ipv4Address(Address(), Port(0));
            error = ::bind(impl.Get(), (sockaddr *)&local, sizeof(local));
            if (error == SOCKET_ERROR)
                throw IocpException(CONNECT_BIND_LOCAL_ERROR);

            sockaddr_in end_point = Ipv4Address(address, port);
            error = ConnectEx(impl.Get(), (sockaddr *)&end_point, sizeof(end_point), 0, 0, 0, ptr.Get());
            if (!error && ::WSAGetLastError() != ERROR_IO_PENDING)
                throw IocpException(CALL_CONNECTEX_FUNCTION_ERROR);

            ptr.Release();
        }

        template<typename ImplementType, typename Buffer, typename Handler>
        void AsyncReceive(ImplementType impl, Buffer& buffer, Handler handler)
        {
            DWORD flags = 0;
            OverlappedPtr<ReceiveOverlapped> ptr(new ReceiveOverlapped(handler, buffer));

            int error = ::WSARecv(impl.Get(), ptr->GetWsaBuf(), ptr->GetWsaBufCount(),
                                  0, &flags, (LPWSAOVERLAPPED)ptr.Get(), 0);
            if (error == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
                throw IocpException(CALL_WSARECV_FUNCTION_ERROR);

            ptr.Release();
        }

        template<typename ImplementType, typename Buffer, typename Handler>
        void AsyncSend(ImplementType impl, const Buffer& buffer, Handler handler)
        {
            OverlappedPtr<SendOverlapped> ptr(new SendOverlapped(handler, buffer));

            int error = ::WSASend(impl.Get(), ptr->GetWsaBuf(), ptr->GetWsaBufCount(),
                                  0, 0, (LPWSAOVERLAPPED)ptr.Get(), 0);
            if (error == SOCKET_ERROR && ::WSAGetLastError() != WSA_IO_PENDING)
                throw IocpException(CALL_WSASEND_FUNCTION_ERROR);

            ptr.Release();
        }
    };

} // namespace net
} // namespace bittorrent

#endif // IOCP_H

#ifndef SOCKET_H
#define SOCKET_H

#include "Iocp.h"
#include <WinSock2.h>

namespace bittorrent {
namespace net {

    template<typename ImplementType>
    class Socket
    {
    public:
        typedef ImplementType implement_type;

        explicit Socket(IocpService& iocp)
            : iocp_(iocp),
              implement_(iocp)
        {
        }

        template<typename Handler>
        void AsyncConnect(Handler handler);

        template<typename Handler>
        void AsyncReceive(Handler handler);

        template<typename Handler>
        void AsyncSend(Handler handler);

    private:
        IocpService& iocp_;
        implement_type implement_;
    };

    template<typename ImplementType>
    class Listener
    {
    public:
        typedef ImplementType implement_type;

        Listener(const Address& address, const Port& port, IocpService& iocp)
            : address_(address),
              port_(port),
              iocp_(iocp),
              implement_(iocp)
        {
        }

        template<typename Handler>
        void AsyncAccept(Handler handler)
        {
            iocp_.AsyncAccept(implement_, handler);
        }

    private:
        Address address_;
        Port port_;
        IocpService& iocp_;
        implement_type implement_;
    };

} // namespace net
} // namespace bittorrent

#endif // SOCKET_H

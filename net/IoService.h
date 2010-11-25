#ifndef IO_SERVICE_H
#define IO_SERVICE_H

#include "Address.h"
#include "Socket.h"
#include "SocketImpl.h"
#include "Overlapped.h"
#include "Iocp.h"

namespace bittorrent {
namespace net {

    typedef IocpService IoService;
    typedef Socket<SocketImpl, IoService> SocketHandler;
    typedef Listener<SocketImpl, IoService> ListenerHandler;

    // a help function to construct a SocketHandler by service and SocketImpl
    inline SocketHandler MakeSocketHandler(IoService& service,
                                           const SocketImpl& impl)
    {
        return SocketHandler(service, impl);
    }

} // namespace net
} // namespace bittorrent

#endif // IO_SERVICE_H

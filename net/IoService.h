#ifndef IO_SERVICE_H
#define IO_SERVICE_H

#include "Address.h"
#include "Socket.h"
#include "BaseSocket.h"
#include "Overlapped.h"
#include "Iocp.h"

namespace bittorrent {
namespace net {

    typedef IocpService IoService;
    typedef Socket<BaseSocket, IoService> AsyncSocket;
    typedef Listener<BaseSocket, IoService> AsyncListener;

    // a help function to construct a AsyncSocket by service and BaseSocket
    inline AsyncSocket MakeAsyncSocket(IoService& service,
                                       const BaseSocket& socket)
    {
        return AsyncSocket(service, socket);
    }

} // namespace net
} // namespace bittorrent

#endif // IO_SERVICE_H

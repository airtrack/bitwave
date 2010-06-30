#ifndef _SOCKET_MANAGER_H_
#define _SOCKET_MANAGER_H_

#include <WinSock2.h>

namespace bittorrent
{
    class SocketManager
    {
    public:
        SocketManager();
        
        // get a new socket
        SOCKET NewSocket();

        // free socket
        void FreeSocket(SOCKET sock);
    };
} // namespace bittorrent

#endif // _SOCKET_MANAGER_H_
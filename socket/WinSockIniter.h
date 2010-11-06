#ifndef WIN_SOCK_INITER_H
#define WIN_SOCK_INITER_H

#include <Winsock2.h>

namespace bittorrent
{
    class WinSockIniter
    {
    public:
        WinSockIniter()
        {
            ::WSAStartup(MAKEWORD(2, 2), &wsadata_);
        }

        ~WinSockIniter()
        {
            ::WSACleanup();
        }

    private:
        WSADATA wsadata_;
    };
} // namespace bittorrent

#endif // WIN_SOCK_INITER_H

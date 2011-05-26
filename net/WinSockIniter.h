#ifndef WIN_SOCK_INITER_H
#define WIN_SOCK_INITER_H

#include <Winsock2.h>

namespace bitwave {
namespace net {

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

} // namespace net
} // namespace bitwave

#endif // WIN_SOCK_INITER_H

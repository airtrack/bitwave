#ifndef OVERLAPPED_H
#define OVERLAPPED_H

#include <functional>
#include <string.h>
#include <WinSock2.h>

namespace bittorrent {
namespace net {

    enum OverlappedType
    {
        ACCEPT,
        CONNECT,
        RECEIVE,
        SEND
    };

    struct Overlapped
    {
        OVERLAPPED overlapped;
        OverlappedType type;

    protected:
        explicit Overlapped(OverlappedType t)
            : type(t)
        {
            memset(&overlapped, 0, sizeof(overlapped));
        }
    };

    struct AcceptOverlapped : public Overlapped
    {
    };

    struct ConnectOverlapped : public Overlapped
    {
    };

    struct ReceiveOverlapped : public Overlapped
    {
    };

    struct SendOverlapped : public Overlapped
    {
    };

} // namespace net
} // namespace bittorrent

#endif // OVERLAPPED_H

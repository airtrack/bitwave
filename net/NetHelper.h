#ifndef NET_HELPER_H
#define NET_HELPER_H

#include "BaseSocket.h"
#include <assert.h>
#include <WinSock2.h>

namespace bitwave {
namespace net {

    inline unsigned long HostToNetl(unsigned long l)
    {
        return ::htonl(l);
    }

    inline long HostToNetl(long l)
    {
        return static_cast<long>(::htonl(l));
    }

    template<typename T>
    inline void HostToNetl(T *ary, int size)
    {
        assert(ary && size > 0);
        for (int i = 0; i < size; ++i)
            ary[i] = HostToNetl(ary[i]);
    }

    inline unsigned int HostToNeti(unsigned int i)
    {
        return static_cast<unsigned int>(::htonl(i));
    }

    inline int HostToNeti(int i)
    {
        return static_cast<int>(::htonl(i));
    }

    template<typename T>
    inline void HostToNeti(T *ary, int size)
    {
        assert(ary && size > 0);
        for (int i = 0; i < size; ++i)
            ary[i] = HostToNeti(ary[i]);
    }

    inline unsigned short HostToNets(unsigned short s)
    {
        return ::htons(s);
    }

    inline short HostToNets(short s)
    {
        return static_cast<short>(::htons(s));
    }

    template<typename T>
    inline void HostToNets(T *ary, int size)
    {
        assert(ary && size > 0);
        for (int i = 0; i < size; ++i)
            ary[i] = HostToNets(ary[i]);
    }

    inline unsigned long NetToHostl(unsigned long l)
    {
        return ::ntohl(l);
    }

    inline long NetToHostl(long l)
    {
        return static_cast<long>(::ntohl(l));
    }

    template<typename T>
    inline void NetToHostl(T *ary, int size)
    {
        assert(ary && size > 0);
        for (int i = 0; i < size; ++i)
            ary[i] = NetToHostl(ary[i]);
    }

    inline unsigned int NetToHosti(unsigned int i)
    {
        return static_cast<unsigned int>(::ntohl(i));
    }

    inline int NetToHosti(int i)
    {
        return static_cast<int>(::ntohl(i));
    }

    template<typename T>
    inline void NetToHosti(T *ary, int size)
    {
        assert(ary && size > 0);
        for (int i = 0; i < size; ++i)
            ary[i] = NetToHosti(ary[i]);
    }

    inline unsigned short NetToHosts(unsigned short s)
    {
        return ::ntohs(s);
    }

    inline short NetToHosts(short s)
    {
        return static_cast<short>(::ntohs(s));
    }

    template<typename T>
    inline void NetToHosts(T *ary, int size)
    {
        assert(ary && size > 0);
        for (int i = 0; i < size; ++i)
            ary[i] = NetToHosts(ary[i]);
    }

    inline unsigned long GetPendingDataSize(const BaseSocket& base_socket)
    {
        unsigned long size = 0;
        ::ioctlsocket(base_socket.Get(), FIONREAD, &size);
        return size;
    }

} // namespace net
} // namespace bitwave

#endif // NET_HELPER_H

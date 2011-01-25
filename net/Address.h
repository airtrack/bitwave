#ifndef ADDRESS_H
#define ADDRESS_H

#include "NetHelper.h"
#include <WinSock2.h>

namespace bittorrent {
namespace net {

    class Address
    {
    public:
        static const unsigned long any = INADDR_ANY;

        Address()
            : address_(HostToNetl(any))
        {
        }

        explicit Address(unsigned long hladdress)
            : address_(HostToNetl(hladdress))
        {
        }

        explicit Address(const char *address)
            : address_(::inet_addr(address))
        {
        }

        explicit Address(const sockaddr_in *addr)
            : address_(addr->sin_addr.s_addr)
        {
        }

        operator unsigned long () const
        {
            return address_;
        }

    private:
        unsigned long address_;
    };

    class Port
    {
    public:
        explicit Port(unsigned short hsport)
            : port_(HostToNets(hsport))
        {
        }

        explicit Port(const sockaddr_in *addr)
            : port_(addr->sin_port)
        {
        }

        operator unsigned short () const
        {
            return port_;
        }

    private:
        unsigned short port_;
    };

    inline sockaddr_in Ipv4Address(const Address& address, const Port& port)
    {
        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = address;
        addr.sin_port = port;
        return addr;
    }

} // namespace net
} // namespace bittorrent

#endif // ADDRESS_H

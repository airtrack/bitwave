#ifndef _ADDRESS_H_
#define _ADDRESS_H_

#include <WinSock2.h>

namespace bittorrent
{
    class Address
    {
    public:
        static const unsigned long any = INADDR_ANY;

        Address()
            : address_(htonl(any))
        {
        }

        explicit Address(unsigned long hladdress)
            : address_(htonl(hladdress))
        {
        }

        explicit Address(const char *address)
            : address_(inet_addr(address))
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
            : port_(htons(hsport))
        {
        }

        operator unsigned short () const
        {
            return port_;
        }

    private:
        unsigned short port_;
    };
} // namespace bittorrent

#endif // _ADDRESS_H_
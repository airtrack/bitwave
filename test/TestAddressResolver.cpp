#include "../net/AddressResolver.h"
#include "../net/WinSockIniter.h"
#include <iostream>

using namespace bittorrent;
using namespace net;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "useage: test addressname" << std::endl;
        return 0;
    }

    WinSockIniter initer;

    try
    {
        ResolveHint hint(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        ResolveResult result = ResolveAddress(argv[1], "", hint);

        std::size_t count = 0;
        for (ResolveResult::iterator it = result.begin();
                it != result.end(); ++it)
        {
            std::cout << "ai_family: " << it->ai_family << std::endl;
            std::cout << "ai_socktype: " << it->ai_socktype << std::endl;
            std::cout << "ai_protocol: " << it->ai_protocol << std::endl;
            std::cout << "ai_canonname: " << (it->ai_canonname ? it->ai_canonname : "") << std::endl;
            std::cout << std::endl;
            ++count;
        }

        std::cout << "there are " << count << " address." << std::endl;
    }
    catch (const AddressResolveException& are)
    {
        std::cerr << "exception EAI code is: " << are.get_exception_code() << std::endl;
        std::cerr << "address is: " << are.what() << std::endl;
    }

    return 0;
}

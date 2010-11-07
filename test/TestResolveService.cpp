#include "../socket/ResolveService.h"
#include "../socket/WinSockIniter.h"
#include <iostream>

using namespace bittorrent;

void AddressResolveHandler(const std::string& nodename, ResolveResult result)
{
    std::cout << nodename << " address info are: " << std::endl;

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

    std::cout << "there are " << count << " address.\n" << std::endl;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "usage: test name1 name2 ... namen" << std::endl;
        return 0;
    }

    WinSockIniter initer;
    ResolveService service;
    ResolveHint hint(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    for (int i = 1; i < argc; ++i)
    {
        service.AsyncResolve(argv[i], "", hint, AddressResolveHandler);
    }

    while (true)
    {
        service.Run();
        std::cout << "waitting ...\n";
        Sleep(100);
    }

    return 0;
}

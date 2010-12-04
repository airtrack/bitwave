#include "../net/ResolveService.h"
#include "../net/WinSockIniter.h"
#include <iostream>

using namespace bittorrent;
using namespace net;

void PrintResult(const std::string& nodename,
                 const std::string& servname,
                 const ResolveResult& result)
{
    std::cout << "\naddress: " << nodename << std::endl;
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

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "usage: test www.sample.com ..." << std::endl;
        return 0;
    }

    WinSockIniter initer;
    ResolveService service;
    ResolveHint hint(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    for (int i = 1; i < argc; ++i)
    {
        service.AsyncResolve(argv[i], "", hint, PrintResult);
    }

    for (int i = 0; i < 50; ++i)
    {
        service.Run();
        Sleep(100);
    }

    std::cout << "\nexiting..." << std::endl;

    return 0;
}

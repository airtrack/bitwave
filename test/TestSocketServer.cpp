#include <iostream>
#include <vector>
#include <string>

#include "../socket/Buffer.h"
#include "../socket/Socket.h"
#include "../socket/IoService.h"
#include "../socket/WinSockIniter.h"

using namespace std;
using namespace bittorrent;

void RecvHandler(SocketHandler& sock)
{
    std::size_t reserved = sock.Reserved();
    
    if (reserved > 0)
    {
        std::vector<char> cvec(reserved);
        sock.Recv(&cvec[0], reserved);
        std::string str(cvec.begin(), cvec.end());
        std::cout << "recvive data from client: " << str << std::endl;
        sock.Send(str.c_str(), str.size());
        std::cout << "send client data back: " << str << std::endl;
    }
}

void AcceptHandler(AcceptorHandler& acceptor, SocketHandler& sock)
{
    std::cout << "accept new client" << std::endl;
    acceptor.AsyncAccept(&AcceptHandler);
}

int main()
{
    WinSockIniter initer;

    IoService service;
    service.RegisterRecvDataCallback(&RecvHandler);
    AcceptorHandler acceptor(service, Port(5150));
    std::cout << "start server: listen port 5150" << std::endl;

    acceptor.AsyncAccept(&AcceptHandler);

    while (true)
    {
        service.Run();
    }

    return 0;
}

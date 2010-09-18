#include <iostream>
#include <vector>
#include <string>

#include "../socket/Buffer.h"
#include "../socket/Socket.h"
#include "../socket/IoService.h"

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
        std::cout << "receive frome server " << str << std::endl;
        sock.Send(str.c_str(), str.size());
        std::cout << "send data: " << str << std::endl;
    }
}

void ConnectHandler(SocketHandler& sock)
{
    std::cout << "connect success" << std::endl;
    const char *msg = "hello socket";
    sock.Send(msg, strlen(msg) + 1);
    std::cout << "send data: " << msg << std::endl;
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    IoService service;
    service.RegisterRecvDataCallback(&RecvHandler);

    SocketHandler conn(service);
    std::cout << "start connecting server" << std::endl;
    conn.AsyncConnect(Address("127.0.0.1"), Port(5150), &ConnectHandler);

    while (true)
    {
        service.Run();
        Sleep(1);
    }

    WSACleanup();

    return 0;
}

#include <iostream>

#include "../socket/Buffer.h"
#include "../socket/Socket.h"
#include "../socket/IoService.h"

using namespace std;
using namespace bittorrent;

DefaultBufferService bufservice;

void RecvHandler(SocketHandler& sock, char *data, std::size_t size, std::size_t received)
{
    std::cout << "recv data is: " << data << std::endl;
    std::cout << "recv data size is: " << received << std::endl;

    Buffer buf(data, size);
    sock.AsyncRecv(buf, &RecvHandler);
}

void AcceptHandler(AcceptorHandler& acceptor, SocketHandler& sock)
{
    std::cout << "accept new client" << std::endl;

    acceptor.AsyncAccept(&AcceptHandler);
    Buffer buf = bufservice.GetBuffer(20);
    sock.AsyncRecv(buf, &RecvHandler);
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    IoService service;
    AcceptorHandler acceptor(service, Port(5150));

    acceptor.AsyncAccept(&AcceptHandler);

    while (true)
        service.Run();

    WSACleanup();

    return 0;
}

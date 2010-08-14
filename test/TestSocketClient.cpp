#include <iostream>

#include "../socket/Buffer.h"
#include "../socket/Socket.h"
#include "../socket/IoService.h"

using namespace std;
using namespace bittorrent;

DefaultBufferService bufservice;

void SendHandler(SocketHandler& sock, char *data, std::size_t size)
{
    std::cout << "send success, data is: " << data << std::endl;
    bufservice.FreeBuffer(data, size);
    sock.Close();
    std::cout << "socket close" << std::endl;
}

void ConnectHandler(SocketHandler& sock)
{
    std::cout << "connect success" << std::endl;
    Buffer data = bufservice.GetBuffer(20);
    strncpy(data.Get(), "hello socket!", 20);
    sock.AsyncSend(data, &SendHandler);
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    IoService service;

    SocketHandler conn(service);
    conn.AsyncConn(Address("127.0.0.1"), Port(5150), &ConnectHandler);

    while (true)
        service.Run();

    WSACleanup();

    return 0;
}

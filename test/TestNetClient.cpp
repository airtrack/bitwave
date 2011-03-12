#include "../net/IoService.h"
#include "../net/WinSockIniter.h"
#include <string.h>
#include <functional>
#include <iostream>

using namespace bittorrent;
using namespace net;

class Buffer
{
public:
    explicit Buffer(int len)
        : buffer_(len)
    {
    }

    char * GetBuffer() const
    {
        return const_cast<char *>(&buffer_[0]);
    }

    int BufferLen() const
    {
        return static_cast<int>(buffer_.size());
    }

private:
    std::vector<char> buffer_;
};

class Client
{
public:
    explicit Client(IoService& service)
        : close_(false),
          socket_(service),
          send_buffer_(100)
    {
        Address server_address("127.0.0.1");
        Port server_port(5150);
        socket_.AsyncConnect(server_address, server_port,
                std::tr1::bind(&Client::ConnectCallback, this, std::tr1::placeholders::_1));
    }

    void ConnectCallback(bool success)
    {
        if (success)
        {
            std::cout << "connect server success\n";

            const char *request = "client request server";
            strncpy(send_buffer_.GetBuffer(), request, send_buffer_.BufferLen());
            socket_.AsyncSend(send_buffer_,
                    std::tr1::bind(&Client::SendCallback, this,
                        std::tr1::placeholders::_1, std::tr1::placeholders::_2));
        }
        else
        {
            std::cout << "connect server failure\n";
            Close();
        }
    }

    void SendCallback(bool success, int send)
    {
        if (success)
        {
            std::cout << "send request success, send: " << send << '\n';
        }
        else
        {
            std::cout << "send request error\n";
        }

        Close();
    }

    bool IsClose() const
    {
        return close_;
    }

    void Close()
    {
        if (!IsClose())
        {
            socket_.Close();
            close_ = true;
        }
    }

private:
    bool close_;
    AsyncSocket socket_;
    Buffer send_buffer_;
};

int main()
{
    WinSockIniter initer;
    IoService io_service;
    Client client(io_service);

    int i = 0;
    while (true)
    {
        io_service.Run();
        if (client.IsClose())
            break;
        ::Sleep(100);
    }

    std::cout << "exiting..." << std::endl;

    return 0;
}

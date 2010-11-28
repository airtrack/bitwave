#include "../net/IoService.h"
#include "../net/WinSockIniter.h"
#include <string.h>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

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

class Connection
{
public:
    Connection(int number, IoService& service, const SocketImpl& new_connect)
        : number_(number),
          socket_handler_(service, new_connect),
          receive_buffer_(100),
          send_buffer_(100)
    {
        Receive();
    }

private:
    void Receive()
    {
        socket_handler_.AsyncReceive(receive_buffer_,
                std::tr1::bind(&Connection::ReceiveCallback, this,
                    std::tr1::placeholders::_1, std::tr1::placeholders::_2));
    }

    void Send()
    {
        const char *respond = "Respond";
        strncpy(send_buffer_.GetBuffer(), respond, send_buffer_.BufferLen());

        socket_handler_.AsyncSend(send_buffer_,
                std::tr1::bind(&Connection::SendCallback, this,
                    std::tr1::placeholders::_1, std::tr1::placeholders::_2));
    }

    void ReceiveCallback(bool success, int received)
    {
        if (success)
        {
            std::cout << "No." << number_ << " Connection receive success, received: "
                      << received << "\n\t" << "data is: " << receive_buffer_.GetBuffer() << '\n';
            Send();
            Receive();
        }
        else
        {
            std::cout << "No." << number_ << " Connection receive error!\n\t"
                      << "No." << number_ << " Connection close!\n";
            socket_handler_.Close();
        }
    }

    void SendCallback(bool success, int send)
    {
        if (success)
        {
            std::cout << "No." << number_ << " Connection send success, send: "
                      << send << '\n';
        }
        else
        {
            std::cout << "No." << number_ << " Connection send error!\n\t"
                      << "No." << number_ << " Connection close!\n";
            socket_handler_.Close();
        }
    }

    std::size_t number_;
    SocketHandler socket_handler_;
    Buffer receive_buffer_;
    Buffer send_buffer_;
};

class Server
{
public:
    explicit Server(IoService& service, const Address& address, const Port& port)
        : service_(service),
          listen_handler_(address, port, service),
          connection_list_()
    {
        Accept();
    }

private:
    typedef std::tr1::shared_ptr<Connection> ConnectionPtr;
    typedef std::vector<ConnectionPtr> ConnectionList;

    void Accept()
    {
        listen_handler_.AsyncAccept(
                std::tr1::bind(&Server::AcceptCallback, this,
                    std::tr1::placeholders::_1, std::tr1::placeholders::_2));
    }

    void AcceptCallback(bool success, SocketImpl new_connect)
    {
        if (success)
        {
            std::size_t number = connection_list_.size() + 1;
            ConnectionPtr ptr(new Connection(number, service_, new_connect));
            connection_list_.push_back(ptr);
            std::cout << "accept new Connection success, No."
                      << number << '\n';
        }
        else
        {
            std::cout << "accept new Connection error!\n";
        }

        Accept();
    }

    IoService& service_;
    ListenerHandler listen_handler_;
    ConnectionList connection_list_;
};

int main()
{
    WinSockIniter initer;
    IoService service;
    Server server(service, Address(), Port(5150));

    while (true)
    {
        service.Run();
        Sleep(100);
    }

    return 0;
}

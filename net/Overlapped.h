#ifndef OVERLAPPED_H
#define OVERLAPPED_H

#include "SocketImpl.h"
#include "../base/BaseTypes.h"
#include <functional>
#include <string.h>
#include <WinSock2.h>

namespace bittorrent {
namespace net {

    enum OverlappedType
    {
        ACCEPT,
        CONNECT,
        RECEIVE,
        SEND
    };

    struct Overlapped
    {
        OVERLAPPED overlapped;
        OverlappedType type;

    protected:
        explicit Overlapped(OverlappedType t)
            : type(t)
        {
            memset(&overlapped, 0, sizeof(overlapped));
        }
    };

    // an Overlapped for iocp service AsyncAccept
    class AcceptOverlapped : public Overlapped
    {
    public:
        template<typename Service>
        AcceptOverlapped(Handler handler, Service& service)
            : Overlapped(ACCEPT),
              handler_(handler),
              accept_socket_(service)
        {
        }

        SOCKET GetAcceptSocket() const
        {
            return accept_socket_.Get();
        }

        char * GetAddressBuf() const
        {
            return address_buffer;
        }

        int GetAddressLength() const
        {
            return address_length;
        }

    private:
        static const int address_length = sizeof(sockaddr_in) + 16;
        static const int address_buffer_size = 2 * address_length;

        Handler handler_;
        SocketImpl accept_socket_;
        char address_buffer[address_buffer_size];
    };

    // an Overlapped for iocp service AsyncConnect
    class ConnectOverlapped : public Overlapped
    {
    public:
        explicit ConnectOverlapped(Handler handler)
            : Overlapped(CONNECT),
              handler_(handler)
        {
        }

    private:
        Handler handler_;
    };

    // an Overlapped for iocp service AsyncReceive
    class ReceiveOverlapped : public Overlapped
    {
        friend void AssignOverlappedBytes(Overlapped *overlapped, int bytes);
    public:
        template<typename Buffer>
        ReceiveOverlapped(Handler handler, Buffer& buffer)
            : Overlapped(RECEIVE),
              handler_(handler),
              wsabuf_(),
              bytes_(0)
        {
            wsabuf_.buf = buffer.GetBuffer();
            wsabuf_.len = buffer.BufferLen();
        }

        LPWSABUF GetWsaBuf()
        {
            return &wsabuf_;
        }

        DWORD GetWsaBufCount() const
        {
            return 1;
        }

    private:
        Handler handler_;
        WSABUF wsabuf_;
        int bytes_;
    };

    // an Overlapped for iocp service AsyncSend
    class SendOverlapped : public Overlapped
    {
        friend void AssignOverlappedBytes(Overlapped *overlapped, int bytes);
    public:
        template<typename Buffer>
        SendOverlapped(Handler handler, const Buffer& buffer)
            : Overlapped(SEND),
              handler_(handler),
              wsabuf_(),
              bytes_(0)
        {
            wsabuf_.buf = buffer.GetBuffer();
            wsabuf_.len = buffer.BufferLen();
        }

        LPWSABUF GetWsaBuf()
        {
            return &wsabuf_;
        }

        DWORD GetWsaBufCount() const
        {
            return 1;
        }

    private:
        Handler handler_;
        WSABUF wsabuf_;
        int bytes_;
    };

    // an exception safe Overlappeds helper template class
    template<typename Type>
    class OverlappedPtr : private NotCopyable
    {
    public:
        explicit OverlappedPtr(Type *overlapped)
            : overlapped_(overlapped)
        {
        }

        ~OverlappedPtr()
        {
            if (!overlapped_)
                return ;
            delete overlapped_;
        }

        OVERLAPPED * Get() const
        {
            return (OVERLAPPED *)overlapped_;
        }

        void Release()
        {
            overlapped_ = 0;
        }

        Type * operator -> () const
        {
            return overlapped_;
        }

        Type& operator * () const
        {
            return *overlapped_;
        }

    private:
        Type *overlapped_;
    };

    namespace overlapped_operation {

        void AssignOverlappedBytes(Overlapped *overlapped, int bytes)
        {
            if (overlapped->type == RECEIVE)
            {
                reinterpret_cast<ReceiveOverlapped *>(overlapped)->bytes_ = bytes;
            }
            else if (overlapped->type == SEND)
            {
                reinterpret_cast<SendOverlapped *>(overlapped)->bytes_ = bytes;
            }
        }

        void InvokeOverlapped(Overlapped *overlapped)
        {
        }

        void DeleteOverlapped(Overlapped *overlapped)
        {
            switch (overlapped->type)
            {
            case ACCEPT:
                delete reinterpret_cast<AcceptOverlapped *>(overlapped);
                break;
            case CONNECT:
                delete reinterpret_cast<ConnectOverlapped *>(overlapped);
                break;
            case RECEIVE:
                delete reinterpret_cast<ReceiveOverlapped *>(overlapped);
                break;
            case SEND:
                delete reinterpret_cast<SendOverlapped *>(overlapped);
                break;
            }
        }

    } // namespace overlapped_operation

} // namespace net
} // namespace bittorrent

#endif // OVERLAPPED_H

#ifndef SOCKET_IMPL_H
#define SOCKET_IMPL_H

#include "../base/RefCount.h"
#include <WinSock2.h>

namespace bittorrent {
namespace net {

    class CreateSocketError {};

    class SocketImpl : public RefCount
    {
    public:
        template<typename Service>
        explicit SocketImpl(Service& service)
            : RefCount(true),
              socket_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        {
            if (socket_ == INVALID_SOCKET)
                throw CreateSocketError();

            service.RegisterSocket(socket_);
        }

        ~SocketImpl()
        {
            if (Only())
                Close();
        }

        SocketImpl(const SocketImpl& si)
            : RefCount(si),
              socket_(si.socket_)
        {
        }

        SocketImpl& operator = (const SocketImpl& si)
        {
            SocketImpl(si).Swap(*this);
            return *this;
        }

        void Swap(SocketImpl& si)
        {
            RefCount::Swap(si);
            std::swap(si.socket_, socket_);
        }

        void Close()
        {
            if (socket_ == INVALID_SOCKET)
                return ;

            ::closesocket(socket_);
            socket_ = INVALID_SOCKET;
        }

        SOCKET Get() const
        {
            return socket_;
        }

    private:
        SOCKET socket_;
    };

} // namespace net
} // namespace bittorrent

#endif // SOCKET_IMPL_H

#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include "../base/RefCount.h"
#include <WinSock2.h>

namespace bitwave {
namespace net {

    class CreateSocketError {};

    class BaseSocket : public RefCount
    {
    public:
        template<typename Service>
        explicit BaseSocket(Service& service)
            : RefCount(true),
              socket_(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
        {
            if (socket_ == INVALID_SOCKET)
                throw CreateSocketError();

            service.RegisterSocket(socket_);
        }

        ~BaseSocket()
        {
            if (Only())
                Close();
        }

        BaseSocket(const BaseSocket& si)
            : RefCount(si),
              socket_(si.socket_)
        {
        }

        BaseSocket& operator = (const BaseSocket& si)
        {
            BaseSocket(si).Swap(*this);
            return *this;
        }

        void Swap(BaseSocket& si)
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
} // namespace bitwave

#endif // BASE_SOCKET_H

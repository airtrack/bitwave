#ifndef IOCP_H
#define IOCP_H

#include "../base/BaseTypes.h"

namespace bittorrent {
namespace net {

    class Iocp : private NotCopyable
    {
    public:
        void Breath();

        template<typename Handler>
        void AsyncAccept(Handler handler);

        template<typename Handler>
        void AsyncConnect(const Address& address, const Port& port, Handler handler);

        template<typename Buffer, typename Handler>
        void AsyncReceive(Buffer& buffer, Handler handler);

        template<typename Buffer, typename Handler>
        void AsyncSend(const Buffer& buffer, Handler handler);
    };

} // namespace net
} // namespace bittorrent

#endif // IOCP_H

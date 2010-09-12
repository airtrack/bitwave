#include "ISocketStream.h"
#include "Socket.h"
#include <assert.h>

namespace bittorrent
{
    // static
    void ISocketStream::RecvCallback(SocketHandler& socket,
            Buffer& buffer, std::size_t received)
    {
        ISocketStream *stream = socket.istream_;
        std::size_t size = stream->streambuf_.putn(buffer.Get(), received);
        assert(size == received);
        stream->recving_ = false;
    }

    void ISocketStream::Recv()
    {
        if (recving_ || streambuf_.full())
            return ;

        std::size_t rsize = streambuf_.reserved();
        Buffer buffer = ioservice_.CreateBuffer(rsize);
        ioservice_.AsyncRecv(socket_, buffer, &ISocketStream::RecvCallback);
        recving_ = true;
    }
} // namespace bittorrent

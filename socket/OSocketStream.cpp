#include "OSocketStream.h"
#include <string.h>

namespace bittorrent
{
    void OSocketStream::Write(const char *data, std::size_t size)
    {
        Buffer buffer = ioservice_.CreateBuffer(size);
        memcpy(buffer.Get(), data, size);
        Write(buffer);
    }

    void OSocketStream::Write(Buffer& buffer)
    {
        // send data by ioservice_, and set the callback is empty
        ioservice_.AsyncSend(socket_, buffer, 0);
    }
} // namespace bittorrent

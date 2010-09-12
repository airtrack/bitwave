#ifndef _OSOCKET_STREAM_H_
#define _OSOCKET_STREAM_H_

#include "IoService.h"
#include "../base/BaseTypes.h"

namespace bittorrent
{
    class OSocketStream : private NotCopyable
    {
    public:
        OSocketStream(IoService& service, SOCKET socket)
            : ioservice_(service),
              socket_(socket)
        {
        }

        // write data to socket stream
        void Write(const char *data, std::size_t size);

        // write data to socket stream, and the buffer need CreateBuffer
        // by ioservice_
        void Write(Buffer& buffer);

    private:
        IoService& ioservice_;
        SOCKET socket_;
    };
} // namespace bittorrent

#endif // _OSOCKET_STREAM_H_

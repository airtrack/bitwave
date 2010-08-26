#ifndef _OSOCKET_STREAM_H_
#define _OSOCKET_STREAM_H_

#include "SocketStreamBuf.h"
#include "../base/BaseTypes.h"

namespace bittorrent
{
    class OSocketStream : private NotCopyable
    {
    public:
        typedef basic_socketstreambuf<char> SocketStreamBuf;
        typedef typename SocketStreamBuf::size_type size_type;
        typedef typename SocketStreamBuf::int_type int_type;

        OSocketStream()
            : streambuf_()
        {
        }

        // write data to socket stream
        void Write(char *data, size_type size);

        // do real write data to socket stream
        void Flush();

    private:
        SocketStreamBuf streambuf_;
    };
} // namespace bittorrent

#endif // _OSOCKET_STREAM_H_

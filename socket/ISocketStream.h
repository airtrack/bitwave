#ifndef _ISOCKET_STREAM_H_
#define _ISOCKET_STREAM_H_

#include "SocketStreamBuf.h"
#include "../base/BaseTypes.h"

namespace bittorrent
{
    class ISocketStream : private NotCopyable
    {
    public:
        typedef basic_socketstreambuf<char> SocketStreamBuf;
        typedef typename SocketStreamBuf::size_type size_type;
        typedef typename SocketStreamBuf::int_type int_type;

        ISocketStream()
            : streambuf_()
        {
        }

        // peek a char from stream, return EOF if no data in stream
        int_type Peek() const
        {
            return streambuf_.peek();
        }

        // peek n number of char, return the real peeked number
        size_type Peekn(char *peeked, size_type n)
        {
            return streambuf_.peekn(peeked, n);
        }

        // read size of char, return the real read size
        size_type Read(char *buf, size_type size)
        {
            return streambuf_.getn(buf, size);
        }

        // return number of char can be read
        size_type Reserved() const
        {
            return streambuf_.buffered();
        }

        // no data in stream
        bool eof() const
        {
            return streambuf_.eof();
        }

    private:
        SocketStreamBuf streambuf_;
    };
} // namespace bittorrent

#endif // _ISOCKET_STREAM_H_

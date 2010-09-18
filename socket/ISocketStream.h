#ifndef _ISOCKET_STREAM_H_
#define _ISOCKET_STREAM_H_

#include "IoService.h"
#include "SocketStreamBuf.h"
#include "../base/BaseTypes.h"

namespace bittorrent
{
    class SocketHandler;

    class ISocketStream : private NotCopyable
    {
    public:
        typedef basic_socketstreambuf<char> SocketStreamBuf;
        typedef SocketStreamBuf::size_type size_type;
        typedef SocketStreamBuf::int_type int_type;

        ISocketStream(IoService& service, SOCKET socket)
            : ioservice_(service),
              socket_(socket),
              streambuf_(),
              recving_(false)
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
            size = streambuf_.getn(buf, size);
            Recv();
            return size;
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

        // pending a async recv
        void Recv();

    private:
        static void RecvCallback(SocketHandler&, Buffer& buffer, std::size_t);

        IoService& ioservice_;
        SOCKET socket_;
        SocketStreamBuf streambuf_;

        // has been pending a recv when it is true
        bool recving_;
    };
} // namespace bittorrent

#endif // _ISOCKET_STREAM_H_

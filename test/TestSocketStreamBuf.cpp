#include "../socket/SocketStreamBuf.h"
#include <iostream>
#include <assert.h>

using namespace bittorrent;

int main()
{
    typedef basic_socketstreambuf<char> SocketStreamBuf;
    SocketStreamBuf buf;
    assert(buf.eof());
    assert(buf.buffered() == 0);

    char data[128] = "just test basic_socketstreambuf";
    std::cout << "data is: " << data << std::endl;
    for (std::size_t i = 0; i < default_streambuf_size / sizeof(data); ++i)
    {
        assert(buf.putn(data, sizeof(data)) == sizeof(data));
        assert(buf.buffered() == sizeof(data) * (i + 1));
    }
    assert(buf.full());

    std::cout << "the first char of buf is: " << static_cast<char>(buf.peek()) << std::endl;
    assert(buf.full());

    char peeked[21] = { 0 };
    assert(buf.peekn(peeked, 20) == 20);
    std::cout << "peek 20 char is: " << peeked << std::endl;
    assert(buf.full());

    for (std::size_t i = 0; i < default_streambuf_size / sizeof(data); ++i)
    {
        assert(buf.getn(data, sizeof(data)) == sizeof(data));
        assert(buf.reserved() == sizeof(data) * (i + 1));
    }
    assert(buf.eof());
    std::cout << "the getted buffer data is: " << data << std::endl;

    return 0;
}

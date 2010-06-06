#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <cstddef>

namespace bittorrent
{
    namespace socket
    {
        struct Buffer
        {
            Buffer()
                : buf(0), buflen(0), used(0) { }

            Buffer(char *b, std::size_t bl)
                : buf(b), buflen(bl), used(0) { }

            char *buf;
            std::size_t buflen;
            std::size_t used;
        };

        class BufferAllocator
        {
        public:
            static Buffer AllocBuf(std::size_t size);
            static void DeallocBuf(Buffer& buf);
        };
    } // namespace socket
} // namespace bittorrent

#endif // _BUFFER_H_
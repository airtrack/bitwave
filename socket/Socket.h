#ifndef _SOCKET_H_
#define _SOCKET_H_

namespace bittorrent
{
    namespace socket
    {
        struct Buffer
        {
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

        class Socket
        {
        public:
            Socket();

            void Send(Buffer& buf);
            void Recv(Buffer& buf);
        };

        class Acceptor
        {
        public:
            Acceptor();
        };
    } // namespace socket
} // namespace bittorrent

#endif // _SOCKET_H_
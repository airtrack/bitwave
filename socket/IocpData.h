#ifndef _IOCP_DATA_H_
#define _IOCP_DATA_H_

namespace bittorrent
{
    struct CompletionKey
    {

    };

    struct Overlapped
    {

    };

    class IocpData
    {
    public:
        IocpData();

        CompletionKey * NewCompletionKey();
        void FreeCompletionKey(CompletionKey *ck);

        Overlapped * NewOverlapped();
        void FreeOverlapped(Overlapped *ol);
    };
} // namespace bittorrent

#endif // _IOCP_DATA_H_
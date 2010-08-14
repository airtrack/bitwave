#ifndef _THREAD_H_
#define _THREAD_H_

#include <Windows.h>
#include <process.h>
#include "../base/BaseTypes.h"

namespace bittorrent
{
    typedef unsigned (__stdcall * ThreadFunctionType) (void *);

    class Thread : private NotCopyable
    {
    public:
        Thread(ThreadFunctionType fun, void *funarg)
            : fun_(fun),
              funarg_(funarg),
              handle_(0)
        {
            StartThread();
        }

        ~Thread()
        {
            CloseHandle((HANDLE)handle_);
        }

    private:
        void StartThread()
        {
            handle_ = _beginthreadex(0, 0, fun_, funarg_, 0, 0);
        }

        ThreadFunctionType fun_;
        void *funarg_;
        unsigned handle_;
    };
} // namespace bittorrent

#endif // _THREAD_H_

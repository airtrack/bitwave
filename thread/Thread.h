#ifndef THREAD_H
#define THREAD_H

#include "../base/BaseTypes.h"
#include <functional>
#include <Windows.h>
#include <process.h>

namespace bittorrent {

    class Thread : private NotCopyable
    {
    public:
        typedef std::tr1::function<unsigned ()> thread_function;

        explicit Thread(const thread_function& fun)
            : thread_fun_(fun),
              handle_(0)
        {
            StartThread();
        }

        ~Thread()
        {
            CloseHandle((HANDLE)handle_);
        }

        HANDLE GetHandle() const
        {
            return (HANDLE)handle_;
        }

    private:
        static unsigned __stdcall ThreadFunction(void *arg)
        {
            thread_function *thread_fun = reinterpret_cast<thread_function *>(arg);
            return (*thread_fun)();
        }

        void StartThread()
        {
            handle_ = _beginthreadex(0, 0, ThreadFunction, &thread_fun_, 0, 0);
        }

        thread_function thread_fun_;
        unsigned handle_;
    };

} // namespace bittorrent

#endif // THREAD_H

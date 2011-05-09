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
              handle_(INVALID_HANDLE_VALUE)
        {
            StartThread();
        }

        ~Thread()
        {
            ::CloseHandle(handle_);
        }

        void Join()
        {
            ::WaitForSingleObject(handle_, INFINITE);
        }

        HANDLE GetHandle() const
        {
            return handle_;
        }

    private:
        static unsigned __stdcall ThreadFunction(void *arg)
        {
            thread_function *thread_fun = reinterpret_cast<thread_function *>(arg);
            return (*thread_fun)();
        }

        void StartThread()
        {
            handle_ = (HANDLE)::_beginthreadex(0, 0, ThreadFunction, &thread_fun_, 0, 0);
        }

        thread_function thread_fun_;
        HANDLE handle_;
    };

} // namespace bittorrent

#endif // THREAD_H

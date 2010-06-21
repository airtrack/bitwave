#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <Windows.h>
#include <process.h>
#include "../base/BaseTypes.h"

namespace bittorrent
{
    namespace thread
    {
        template<int spincount = 4000>
        struct InitMutexWithSpinlocks
        {
            static void Init(CRITICAL_SECTION *cs)
            {
                ::InitializeCriticalSectionAndSpinCount(cs, spincount);
            }
        };

        struct InitMutexWithNormal
        {
            static void Init(CRITICAL_SECTION *cs)
            {
                ::InitializeCriticalSection(cs);
            }
        };

        template<typename InitMethod>
        class Mutex : private NotCopyable
        {
            friend template<> class Locker<InitMethod>;
        public:
            Mutex()
            {
                InitMethod::Init(&cs);
            }

            ~Mutex()
            {
                ::DeleteCriticalSection(&cs);
            }

        private:
            void Lock()
            {
                ::EnterCriticalSection(&cs);
            }

            void UnLock()
            {
                ::LeaveCriticalSection(&cs);
            }

            CRITICAL_SECTION cs;
        };

        typedef Mutex<InitMutexWithNormal> NormalMutex;
        typedef Mutex<InitMutexWithSpinlocks<> > SpinlocksMutex;

        template<typename MutexInitMethod>
        class Locker : private NotCopyable
        {
        public:
            explicit Locker(Mutex<MutexInitMethod>& m) : m_(m)
            {
                m_.Lock();
            }

            ~Locker()
            {
                m_.UnLock();
            }

        private:
            Mutex<MutexInitMethod>& m_;
        };

        typedef Locker<InitMutexWithNormal> NormalMutexLocker;
        typedef Locker<InitMutexWithSpinlocks<> > SpinlocksMutexLocker;
    } // namespace thread
} // namespace bittorrent

#endif // _MUTEX_H_
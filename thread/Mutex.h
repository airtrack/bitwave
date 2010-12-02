#ifndef MUTEX_H
#define MUTEX_H

#include "../base/BaseTypes.h"
#include <Windows.h>
#include <process.h>

namespace bittorrent {

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
    public:
        Mutex()
        {
            InitMethod::Init(&cs);
        }

        ~Mutex()
        {
            ::DeleteCriticalSection(&cs);
        }

        void Lock()
        {
            ::EnterCriticalSection(&cs);
        }

        void UnLock()
        {
            ::LeaveCriticalSection(&cs);
        }

    private:
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

    template<typename MutexType>
    struct LockerType;

    template<>
    struct LockerType<NormalMutex>
    {
        typedef NormalMutexLocker type;
    };

    template<>
    struct LockerType<SpinlocksMutex>
    {
        typedef SpinlocksMutexLocker type;
    };

} // namespace bittorrent

#endif // MUTEX_H

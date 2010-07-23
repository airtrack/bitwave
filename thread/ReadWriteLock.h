#ifndef _READ_WRITE_H_
#define _READ_WRITE_H_

#include <Windows.h>
#include <process.h>
#include "../base/BaseTypes.h"

namespace bittorrent
{
    class ReadWriteLock : private NotCopyable
    {
    public:
        ReadWriteLock()
        {
            ::InitializeSRWLock(&lock_);
        }

        void LockForRead()
        {
            ::AcquireSRWLockShared(&lock_);
        }

        void UnLockForRead()
        {
            ::ReleaseSRWLockShared(&lock_);
        }

        void LockForWrite()
        {
            ::AcquireSRWLockExclusive(&lock_);
        }

        void UnLockForWrite()
        {
            ::ReleaseSRWLockExclusive(&lock_);
        }

    private:
        SRWLOCK lock_;
    };

    class ReadLocker : private NotCopyable
    {
    public:
        explicit ReadLocker(ReadWriteLock& lock)
            : lock_(lock)
        {
            lock_.LockForRead();
        }

        ~ReadLocker()
        {
            lock_.UnLockForRead();
        }

    private:
        ReadWriteLock& lock_;
    };

    class WriteLocker : private NotCopyable
    {
    public:
        explicit WriteLocker(ReadWriteLock& lock)
            : lock_(lock)
        {
            lock_.LockForWrite();
        }

        ~WriteLocker()
        {
            lock_.UnLockForWrite();
        }

    private:
        ReadWriteLock& lock_;
    };
} // namespace bittorrent

#endif // _READ_WRITE_H_

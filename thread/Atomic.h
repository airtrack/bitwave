#ifndef ATOMIC_H
#define ATOMIC_H

#include <Windows.h>

namespace bitwave {

    inline long AtomicIncrement(long volatile *addend)
    {
        return ::InterlockedIncrement(addend);
    }

    inline long AtomicDecrement(long volatile *addend)
    {
        return ::InterlockedDecrement(addend);
    }

    inline long AtomicAdd(long volatile *addend, long value)
    {
        return ::InterlockedExchangeAdd(addend, value);
    }

} // namespace bitwave

#endif // ATOMIC_H

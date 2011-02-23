#ifndef TIME_TRAITS_H
#define TIME_TRAITS_H

#include <Windows.h>

namespace bittorrent {

    template<typename TimeType>
    struct time_traits;

    template<>
    struct time_traits<DWORD>
    {
        static bool less_equal(DWORD left, DWORD right)
        {
            return left <= right;
        }

        static DWORD add(DWORD base, int millisecond)
        {
            if (millisecond >= 0)
                return base + millisecond;
            else
                return base - (-millisecond);
        }

        static DWORD invalid()
        {
            return -1;
        }

        static DWORD now()
        {
            return ::GetTickCount();
        }
    };

} // namespace bittorrent

#endif // TIME_TRAITS_H

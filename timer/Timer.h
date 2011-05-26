#ifndef TIMER_H
#define TIMER_H

#include "TimeTraits.h"
#include "../base/BaseTypes.h"
#include <assert.h>
#include <functional>

namespace bitwave {

    template<typename TimeType>
    class BasicTimer : private NotCopyable
    {
    public:
        typedef time_traits<TimeType> TimeTraits;
        typedef std::tr1::function<void ()> TimerCallback;

        BasicTimer()
            : deadline_(TimeTraits::invalid())
        {
        }

        explicit BasicTimer(int millisecond)
        {
            SetDeadline(millisecond);
        }

        void Schedule(const TimeType& now)
        {
            if (TimeTraits::less_equal(deadline_, now))
            {
                assert(callback_);
                callback_();
            }
        }

        void SetCallback(const TimerCallback& callback)
        {
            callback_ = callback;
        }

        void SetDeadline(int millisecond)
        {
            TimeType now = TimeTraits::now();
            deadline_ = TimeTraits::add(now, millisecond);
        }

    private:
        TimeType deadline_;
        TimerCallback callback_;
    };

    typedef BasicTimer<DWORD> Timer;

} // namespace bitwave

#endif // TIMER_H

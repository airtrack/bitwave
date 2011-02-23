#ifndef TIMER_QUEUE_H
#define TIMER_QUEUE_H

#include "Timer.h"
#include "../base/BaseTypes.h"
#include <set>

namespace bittorrent {

    template<typename TimeType>
    class BasicTimerQueue : private NotCopyable
    {
    public:
        typedef BasicTimer<TimeType> TimerImpl;
        typedef typename TimerImpl::TimeTraits TimeTraits;
        typedef std::set<TimerImpl *> TimerList;

        void AddTimer(TimerImpl *new_timer)
        {
            timers_.insert(new_timer);
        }

        void DelTimer(TimerImpl *timer)
        {
            timers_.erase(timer);
        }

        void Schedule()
        {
            TimeType now = TimeTraits::now();
            TimerList::iterator it = timers_.begin();
            while (it != timers_.end())
            {
                TimerImpl *timer = *it++;
                timer->Schedule(now);
            }
        }

    private:
        TimerList timers_;
    };

    typedef BasicTimerQueue<DWORD> TimerQueue;

} // namespace bittorrent

#endif // TIMER_QUEUE_H

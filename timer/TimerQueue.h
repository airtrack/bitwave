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
        typedef BasicTimer<TimeType> TimerType;
        typedef typename TimerType::TimeTraits TimeTraits;
        typedef std::set<TimerType *> TimerList;
        typedef typename TimerList::iterator Iterator;

        BasicTimerQueue()
            : timers_(),
              schedule_it_(timers_.end())
        {
        }

        void AddTimer(TimerType *new_timer)
        {
            timers_.insert(new_timer);
        }

        void DelTimer(TimerType *timer)
        {
            Iterator it = timers_.find(timer);
            if (it != timers_.end())
            {
                if (it == schedule_it_)
                {
                    // keep schedule_it_ validate
                    ++schedule_it_;
                }

                timers_.erase(it);
            }
        }

        void Schedule()
        {
            TimeType now = TimeTraits::now();
            schedule_it_ = timers_.begin();
            while (schedule_it_ != timers_.end())
            {
                TimerType *timer = *schedule_it_++;
                timer->Schedule(now);
            }
        }

    private:
        TimerList timers_;
        Iterator schedule_it_;
    };

    typedef BasicTimerQueue<DWORD> TimerQueue;

} // namespace bittorrent

#endif // TIMER_QUEUE_H

#ifndef TIMER_SERVICE_H
#define TIMER_SERVICE_H

#include "ServiceBase.h"
#include "../timer/TimerQueue.h"

namespace bittorrent {
namespace net {

    class TimerService : public BasicService<TimerService>
    {
    public:
        typedef TimerQueue::TimerType TimerType;

        void AddTimer(TimerType *new_timer)
        {
            timer_queue_.AddTimer(new_timer);
        }

        void DelTimer(TimerType *timer)
        {
            timer_queue_.DelTimer(timer);
        }

    private:
        virtual void DoRun()
        {
            timer_queue_.Schedule();
        }

        TimerQueue timer_queue_;
    };

} // namespace net
} // namespace bittorrent

#endif // TIMER_SERVICE_H

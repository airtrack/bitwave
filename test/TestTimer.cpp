#include "../timer/Timer.h"
#include "../timer/TimerQueue.h"
#include "../unittest/UnitTest.h"
#include <vector>
#include <iostream>
#include <functional>

using namespace bittorrent;

TimerQueue timer_queue;
std::vector<Timer *> dead_timer;

void RecordTimer(Timer *timer)
{
    timer_queue.DelTimer(timer);
    dead_timer.push_back(timer);
}

TEST_CASE(test_timer)
{
    Timer t1(1000);
    t1.SetCallback(std::tr1::bind(&RecordTimer, &t1));
    timer_queue.AddTimer(&t1);

    Timer t2(2000);
    t2.SetCallback(std::tr1::bind(&RecordTimer, &t2));
    timer_queue.AddTimer(&t2);

    Timer t3(5000);
    t3.SetCallback(std::tr1::bind(&RecordTimer, &t3));
    timer_queue.AddTimer(&t3);

    for (int i = 0; i < 100; ++i)
    {
        timer_queue.Schedule();
        ::Sleep(60);
    }

    CHECK_TRUE(dead_timer.size() == 3);
    CHECK_TRUE(dead_timer[0] == &t1);
    CHECK_TRUE(dead_timer[1] == &t2);
    CHECK_TRUE(dead_timer[2] == &t3);
}

int main()
{
    TestCollector.RunCases();

    return 0;
}

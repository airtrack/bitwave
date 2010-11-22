#include "../thread/Thread.h"
#include <stdio.h>
#include <functional>

using namespace bittorrent;

unsigned Threadn(int n)
{
    for (int i = 0; i < 10; ++i)
        printf("print in thread %d\n", n);

    return 0;
}

int main()
{
    Thread t1(std::tr1::bind(Threadn, 1));
    Thread t2(std::tr1::bind(Threadn, 2));
    Thread t3(std::tr1::bind(Threadn, 3));

    Sleep(1000);

    return 0;
}

#include <vector>
#include "base/ObjectPool.h"

struct TestPod
{
    int i;
    char c;
};

int main()
{
    ObjectPool<TestPod> pool;
    std::vector<TestPod *> temp;

    for (int i = 0; i < 1000; ++i)
    {
        temp.push_back(pool.ObtainObject());
    }

    for (int i = 0; i < 1000; ++i)
    {
        pool.ReturnObject(temp[i]);
    }

    return 0;
}

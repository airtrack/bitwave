#include "../unittest/UnitTest.h"

TEST_CASE(test1)
{
    CHECK_TRUE(1 == 1);
    CHECK_TRUE(true == false);
}

TEST_CASE(test2)
{
    CHECK_TRUE(true);
    CHECK_TRUE(1 == 1);
}

int main()
{
    TestCollector.RunCases();

    return 0;
}

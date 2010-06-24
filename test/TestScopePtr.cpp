#include <iostream>
#include "../base/ScopePtr.h"

struct Test
{
    int a_;
    int b_;

    Test(int a, int b)
        : a_(a), b_(b)
    {
    }

    void print() const
    {
        std::cout << "print in Test: " << a_ << " " << b_ << std::endl;
    }
};

int main()
{
    ScopePtr<int> pint(new int(10));
    std::cout << "before assign " << *pint << std::endl;
    *pint = 5;
    std::cout << "after assign " << *pint << std::endl;

    ScopePtr<Test> ptest(new Test(5, 10));
    std::cout << "print out Test: " << ptest->a_ << " " << ptest->b_ << std::endl;
    ptest->print();

    return 0;
}
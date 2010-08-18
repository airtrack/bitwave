#ifndef _REF_COUNT_H_
#define _REF_COUNT_H_

#include <algorithm>

class RefCount
{
public:
    RefCount()
        : usecount_(0)
    {
    }

    RefCount(bool)
        : usecount_(new int(1))
    {
    }

    ~RefCount()
    {
        if (usecount_ && --*usecount_ == 0)
            delete usecount_;
    }

    RefCount(const RefCount& rc)
        : usecount_(0)
    {
        if (rc.UseCount() > 0)
        {
            usecount_ = rc.usecount_;
            ++*usecount_;
        }
    }

    RefCount& operator = (const RefCount& rc)
    {
        RefCount(rc).Swap(*this);
        return *this;
    }

    void Swap(RefCount& rc)
    {
        std::swap(rc.usecount_, usecount_);
    }

    int UseCount() const
    {
        return usecount_ ? *usecount_ : 0;
    }

    bool Only() const
    {
        return UseCount() == 1;
    }

private:
    int *usecount_;
};

#endif // _REF_COUNT_H_

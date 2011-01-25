#ifndef SCOPE_PTR_H
#define SCOPE_PTR_H

#include "BaseTypes.h"

template<typename T>
class ScopePtr : private NotCopyable
{
public:
    explicit ScopePtr(T *pt = 0)
        : pt_(pt)
    {
    }

    ~ScopePtr()
    {
        delete pt_;
    }

    const T *operator -> () const
    {
        return pt_;
    }

    T *operator -> ()
    {
        return pt_;
    }

    const T& operator * () const
    {
        return *pt_;
    }

    T& operator * ()
    {
        return *pt_;
    }

    void Reset(T *pt = 0)
    {
        delete pt_;
        pt_ = pt;
    }

    T * Get() const
    {
        return pt_;
    }

private:
    T *pt_;
};

#endif // SCOPE_PTR_H

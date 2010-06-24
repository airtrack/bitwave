#ifndef _SCOPE_PTR_H_
#define _SCOPE_PTR_H_

#include "BaseTypes.h"

template<typename T>
class ScopePtr : private NotCopyable
{
public:
    ScopePtr(T *pt)
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

private:
    T *pt_;
};

#endif // _SCOPE_PTR_H_
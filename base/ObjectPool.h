#ifndef _OBJECT_POOL_H_
#define _OBJECT_POOL_H_

#include <string.h>
#include <vector>
#include <deque>
#include <algorithm>
#include "BaseTypes.h"

template<typename T>
struct PODIniter
{
    static void Init(T *obj, std::size_t count = 1)
    {
        memset(obj, 0, sizeof(T) * count);
    }
};

template<typename T, typename ObjIniter = PODIniter<T> >
class ObjectPool : private NotCopyable
{
public:
    static const std::size_t basecount = 100;
    static const std::size_t increase = 50;

    ObjectPool()
        : pools_(),
          notusedpool_(),
          basecount_(basecount),
          increase_(increase)
    {
        AllocObjects(basecount_);
    }

    ObjectPool(std::size_t count, std::size_t inccount)
        : pools_(),
          notusedpool_(),
          basecount_(count),
          increase_(inccount)
    {
        AllocObjects(basecount_);
    }

    ~ObjectPool()
    {
        std::for_each(pools_.begin(), pools_.end(), DeleteArray<T>());
    }

    T * ObtainObject()
    {
        if (notusedpool_.empty())
            AllocObjects(increase_);

        T *ret = notusedpool_.front();
        notusedpool_.pop_front();
        return ret;
    }

    void ReturnObject(T *obj)
    {
        ObjIniter::Init(obj);
        notusedpool_.push_back(obj);
    }

private:
    void AllocObjects(std::size_t size)
    {
        T *newobjs = new T[size];
        ObjIniter::Init(newobjs, size);

        pools_.push_back(newobjs);
        T *endnewobjs = newobjs + size;
        for (; newobjs < endnewobjs; ++newobjs)
            notusedpool_.push_back(newobjs);
    }

    std::vector<T *> pools_;
    std::deque<T *> notusedpool_;
    const std::size_t basecount_;
    const std::size_t increase_;
};

#endif // _OBJECT_POOL_H_

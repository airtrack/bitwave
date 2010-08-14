#ifndef _BASE_TYPES_H_
#define _BASE_TYPES_H_

#include <functional>
#include <string>

class NotCopyable
{
public:
    NotCopyable() { }
private:
    NotCopyable(const NotCopyable&);
    NotCopyable& operator = (const NotCopyable&);
};

class BaseException
{
public:
    explicit BaseException(const char *w) : what_(w) { }

    virtual const char *what() const
    {
        return what_.c_str();
    }

private:
    std::string what_;
};

template<typename T>
struct DelObject : public std::unary_function<T *, void>
{
    void operator () (T *obj) const
    {
        delete obj;
    }
};

template<typename T>
struct DelArray : public std::unary_function<T *, void>
{
    void operator () (T *array) const
    {
        delete [] array;
    }
};

struct DeleteSecondOfPair
{
    template<typename PairType>
    void operator () (PairType& pair) const
    {
        delete pair.second;
    }
};

#endif // _BASE_TYPES_H_

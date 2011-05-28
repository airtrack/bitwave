#ifndef BASE_TYPES_H
#define BASE_TYPES_H

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

class StaticClass
{
private:
    StaticClass();
    StaticClass(const StaticClass&);
    StaticClass& operator = (const StaticClass&);
};

class BaseException
{
public:
    explicit BaseException(int ecode)
        : exception_code_(ecode)
    {
    }

    explicit BaseException(const std::string& w)
        : exception_code_(0),
          what_(w)
    {
    }

    BaseException(int ecode, const std::string& w)
        : exception_code_(ecode),
          what_(w)
    {
    }

    int get_exception_code() const
    {
        return exception_code_;
    }

    const char * what() const
    {
        return what_.c_str();
    }

private:
    int exception_code_;
    std::string what_;
};

template<typename T>
inline void CheckedDelete(T *t)
{
    typedef char TypeMustBeComplete[sizeof(T) ? 1 : -1];
    (void)sizeof(TypeMustBeComplete);
    delete t;
}

template<typename T>
inline void CheckedArrayDelete(T *t)
{
    typedef char TypeMustBeComplete[sizeof(T) ? 1 : -1];
    (void)sizeof(TypeMustBeComplete);
    delete [] t;
}

template<typename T>
struct DelObject : public std::unary_function<T *, void>
{
    void operator () (T *obj) const
    {
        CheckedDelete(obj);
    }
};

template<typename T>
struct DelArray : public std::unary_function<T *, void>
{
    void operator () (T *array) const
    {
        CheckedArrayDelete(array);
    }
};

struct DeleteSecondOfPair
{
    template<typename PairType>
    void operator () (PairType& pair) const
    {
        CheckedDelete(pair.second);
    }
};

#endif // BASE_TYPES_H

#ifndef _BASE_TYPES_H_
#define _BASE_TYPES_H_

#include <exception>
#include <string>

class NotCopyable
{
public:
    NotCopyable() { }
private:
    NotCopyable(const NotCopyable&);
    NotCopyable& operator = (const NotCopyable&);
};

class BaseException : public std::exception
{
public:
    BaseException(const char *w) : what_(w) { }

    virtual const char *what() const
    {
        return what_.c_str();
    }

private:
    std::string what_;
};

#endif // _BASE_TYPES_H_

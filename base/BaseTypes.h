#ifndef _BASE_TYPES_H_
#define _BASE_TYPES_H_

class NotCopyable
{
public:
    NotCopyable() { }
private:
    NotCopyable(const NotCopyable&);
    NotCopyable& operator = (const NotCopyable&);
};

#endif // _BASE_TYPES_H_
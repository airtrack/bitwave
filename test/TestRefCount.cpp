#include "../base/RefCount.h"
#include <iostream>

class RefObject : public RefCount
{
public:
    RefObject()
        : RefCount(true),
          data_(new int(10))
    {
    }

    ~RefObject()
    {
        if (Only())
            delete data_;
    }

    RefObject(const RefObject& ro)
        : RefCount(ro),
          data_(ro.data_)
    {
    }

    RefObject& operator = (const RefObject& ro)
    {
        RefObject(ro).Swap(*this);
        return *this;
    }

    void Swap(RefObject& ro)
    {
        RefCount::Swap(ro);
        std::swap(ro.data_, data_);
    }

    int *data_;
};

int main()
{
    RefObject obj;
    std::cout << "use count: " << obj.UseCount() << std::endl;
    std::cout << "obj: " << *obj.data_ << "\n\n";

    {
        RefObject obj1(obj);
        std::cout << "obj use count: " << obj.UseCount() << std::endl;
        std::cout << "obj1 use count: " << obj1.UseCount() << std::endl;
        std::cout << "obj: " << *obj.data_ << std::endl;
        std::cout << "obj1: " << *obj1.data_ << "\n\n";
    }

    {
        RefObject obj2;
        obj2 = obj;
        std::cout << "obj use count: " << obj.UseCount() << std::endl;
        std::cout << "obj2 use count: " << obj2.UseCount() << std::endl;
        std::cout << "obj: " << *obj.data_ << std::endl;
        std::cout << "obj2: " << *obj2.data_ << "\n\n";
    }

    std::cout << "use count: " << obj.UseCount() << std::endl;
    std::cout << "obj: " << *obj.data_ << "\n\n";

    return 0;
}

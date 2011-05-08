#ifndef SHA1_VALUE_H
#define SHA1_VALUE_H

#include <string>
#include <string.h>

namespace bittorrent {

    class Sha1Value
    {
    public:
        Sha1Value();
        explicit Sha1Value(const unsigned *value_ary);
        Sha1Value(const char *begin, const char *end);
        Sha1Value(const char *begin, std::size_t length);

        std::string GetReadableString() const;
        const char * GetData() const;
        int GetDataSize() const;

        friend bool operator == (const Sha1Value& left, const Sha1Value& right)
        {
            return memcmp(left.value_, right.value_, sizeof(left.value_)) == 0;
        }

        friend bool operator != (const Sha1Value& left, const Sha1Value& right)
        {
            return !(left == right);
        }

        friend bool operator < (const Sha1Value& left, const Sha1Value& right)
        {
            return memcmp(left.value_, right.value_, sizeof(left.value_)) < 0;
        }

        friend bool operator > (const Sha1Value& left, const Sha1Value& right)
        {
            return memcmp(left.value_, right.value_, sizeof(left.value_)) > 0;
        }

    private:
        void Calculate(const char *begin, std::size_t length);
        unsigned value_[5];
    };

} // namespace bittorrent

#endif // SHA1_VALUE_H

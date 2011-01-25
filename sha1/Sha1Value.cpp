#include "Sha1Value.h"
#include "sha1.h"
#include <sstream>

namespace bittorrent {

    Sha1Value::Sha1Value()
    {
        memset(value_, 0, sizeof(value_));
    }

    Sha1Value::Sha1Value(const char *begin, const char *end)
    {
        Calculate(begin, end - begin);
    }

    Sha1Value::Sha1Value(const char *begin, std::size_t length)
    {
        Calculate(begin, length);
    }

    std::string Sha1Value::GetReadableString() const
    {
        std::ostringstream oss;
        oss << std::hex;
        for (int i = 0; i < 5; ++i)
            oss << value_[i];
        return oss.str();
    }

    const char * Sha1Value::GetData() const
    {
        return reinterpret_cast<const char *>(value_);
    }

    int Sha1Value::GetDataSize() const
    {
        return sizeof(value_);
    }

    void Sha1Value::Calculate(const char *begin, std::size_t length)
    {
        SHA1 sha1;
        sha1.Input(begin, length);
        sha1.Result(value_);
    }

} // namespace bittorrent

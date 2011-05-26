#include "URI.h"
#include <ctype.h>

namespace {

    // convert lower 4 bits to a hex char
    char X16(int c)
    {
        c &= 0x0F;
        if (c >= 0 && c <= 9)
            return '0' + c;
        c -= 10;
        return 'A' + c;
    }

} // namespace

namespace bitwave {
namespace http {

    void URI::AddQuery(const char *kbegin, const char *kend,
                       const char *vbegin, const char *vend)
    {
        AppendQueryBeginChar();
        AppendEscapeData(kbegin, kend);
        AppendQueryLinkChar();
        AppendEscapeData(vbegin, vend);
        ++numofquery_;
    }

    bool URI::IsUnReserved(char c) const
    {
        return isalnum((unsigned char)c) || c == '.' || c == '_' || c == '~' || c == '-';
    }

    void URI::AppendPercentEncode(char c)
    {
        char str[4] = { 0 };
        str[0] = '%';
        str[1] = X16(c >> 4);
        str[2] = X16(c);
        query_.append(str);
    }

    void URI::AppendQueryLinkChar()
    {
        query_.push_back('=');
    }

    void URI::AppendQueryBeginChar()
    {
        if (numofquery_ == 0)
            query_.push_back('?');
        else
            query_.push_back('&');
    }

    void URI::AppendEscapeData(const char *begin, const char *end)
    {
        for (; begin != end; ++begin)
        {
            if (IsUnReserved(*begin))
                query_.push_back(*begin);
            else
                AppendPercentEncode(*begin);
        }
    }

    void URI::ParseHostFromUri()
    {
        std::string::size_type begin = query_.find("://");
        if (begin == std::string::npos)
            throw InvalidateURI(InvalidateURI::NO_SCHEME);
        else if (begin + 3 >= query_.size())
            throw InvalidateURI(InvalidateURI::NO_AUTHORITY);

        begin += 3;
        std::string::size_type end = query_.find_first_of("/?#", begin);
        if (end == std::string::npos)
        {
            host_ = query_.substr(begin);
            query_ = "/";
        }
        else
        {
            host_ = query_.substr(begin, end - begin);
            query_ = query_.substr(end);
        }

        end = host_.find(':');
        if (end != std::string::npos)
            host_.erase(end, host_.size() - end);
    }

} // namespace http
} // namespace bitwave

#include "URI.h"

namespace bittorrent
{
    void URI::AddQuery(const char *kbegin, const char *kend,
                       const char *vbegin, const char *vend)
    {
        AppendQueryBeginChar();
        AppendEscapeData(kbegin, kend);
        AppendQueryLinkChar();
        AppendEscapeData(vbegin, vend);
        ++numofquery_;
    }

    void URI::AppendQueryBeginChar()
    {
        if (numofquery_ == 0)
            uri_.push_back('?');
        else
            uri_.push_back('&');
    }

    void URI::AppendEscapeData(const char *begin, const char *end)
    {
        for (; begin != end; ++begin)
        {
            if (IsUnReserved(*begin))
                uri_.push_back(*begin);
            else
                AppendPercentEncode(*begin);
        }
    }

    void URI::ParseHostFromUri()
    {
        std::string::size_type begin = uri_.find("://");
        if (begin == std::string::npos)
            throw InvalidateURI(InvalidateURI::NO_SCHEME);
        else if (begin + 3 >= uri_.size())
            throw InvalidateURI(InvalidateURI::NO_AUTHORITY);

        begin += 3;
        std::string::size_type end = uri_.find_first_of("/?#", begin);
        if (end == std::string::npos)
            host_ = uri_.substr(begin);
        else
            host_ = uri_.substr(begin, end - begin);
    }
} // namespace bittorrent

#ifndef URI_H
#define URI_H

#include <string>
#include <sstream>
#include <ctype.h>
#include <stdio.h>

namespace bittorrent
{
    // An exception class to describe an error of URI
    class InvalidateURI
    {
    public:
        enum
        {
            NO_SCHEME,
            NO_AUTHORITY,
        };

        explicit InvalidateURI(int errcode)
            : errcode_(errcode)
        {
        }

        int GetError() const
        {
            return errcode_;
        }

    private:
        int errcode_;
    };

    // a simply URI class to present a uri.
    // use a valid url string to construct a object, then we can get host
    // from the url, and we can add query to URI, then we can get a full uri string
    class URI
    {
    public:
        explicit URI(const std::string& url)
            : numofquery_(0),
              uri_(url),
              host_()
        {
            ParseHostFromUri();
        }

        std::string GetHost() const
        {
            return host_;
        }

        std::string GetURIString() const
        {
            return uri_;
        }

        void AddQuery(const char *kbegin, const char *kend,
                      const char *vbegin, const char *vend);

        template<typename Key, typename Value>
        void AddQuery(const Key& key, const Value& value)
        {
            std::string kstr = TransformToString(key);
            std::string vstr = TransformToString(value);
            AddQuery(kstr.c_str(), kstr.c_str() + kstr.size(), vstr.c_str(), vstr.c_str() + vstr.size());
        }

        template<typename Key>
        void AddQuery(const Key& key, const char *binary, int binlen)
        {
            std::string kstr = TransformToString(key);
            AddQuery(kstr.c_str(), kstr.c_str() + kstr.size(), binary, binary + binlen);
        }

    private:
        template<typename T>
        std::string TransformToString(const T& t) const
        {
            std::ostringstream oss;
            oss << t;
            return oss.str();
        }

        bool IsUnReserved(char c) const
        {
            return isalnum(c) || c == '-' || c == '.' || c == '_' || c == '~';
        }

        void AppendPercentEncode(char c)
        {
            char str[8] = { 0 };
            sprintf(str, "%%%2x", c);
            uri_.append(str);
        }

        void AppendQueryLinkChar() { uri_.push_back('='); }
        void AppendQueryBeginChar();
        void AppendEscapeData(const char *begin, const char *end);
        void ParseHostFromUri();

        int numofquery_;
        std::string uri_;
        std::string host_;
    };
} // namespace bittorrent

#endif // URI_H

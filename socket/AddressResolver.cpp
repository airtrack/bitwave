#include "AddressResolver.h"

namespace bittorrent
{
    ResolveResult::iterator ResolveResult::begin() const
    {
        return iterator(ai_);
    }

    ResolveResult::iterator ResolveResult::end() const
    {
        return iterator();
    }

    ResolveResult ResolveAddress(const std::string& nodename,
                                 const std::string& servname,
                                 const ResolveHint& hint)
    {
        addrinfo *result = 0;
        int ec = ::getaddrinfo(nodename.c_str(), servname.c_str(), hint.Get(), &result);
        if (ec)
            throw AddressResolveException(ec);

        return ResolveResult(result);
    }
} // namespace bittorrent

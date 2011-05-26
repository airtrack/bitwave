#ifndef ADDRESS_RESOLVER_H
#define ADDRESS_RESOLVER_H

#include "../base/RefCount.h"

#include <string>
#include <string.h>
#include <Ws2tcpip.h>

namespace bitwave {
namespace net {

    class ResolveResult : public RefCount
    {
    public:
        class iterator
        {
        public:
            iterator()
                : ai_(0)
            {
            }

            explicit iterator(addrinfo *ai)
                : ai_(ai)
            {
            }

            const addrinfo& operator * () const
            {
                return *ai_;
            }

            const addrinfo* operator -> () const
            {
                return ai_;
            }

            iterator& operator ++ ()
            {
                ai_ = ai_->ai_next;
                return *this;
            }

            iterator operator ++ (int)
            {
                iterator res(*this);
                ++(*this);
                return res;
            }

            friend bool operator == (const iterator& rl, const iterator& rr)
            {
                return rl.ai_ == rr.ai_;
            }

            friend bool operator != (const iterator& rl, const iterator& rr)
            {
                return !(rl == rr);
            }

        private:
            addrinfo *ai_;
        };

        ResolveResult()
            : ai_(0)
        {
        }

        explicit ResolveResult(addrinfo *ai)
            : RefCount(true),
              ai_(ai)
        {
        }

        ~ResolveResult()
        {
            if (Only())
                ::freeaddrinfo(ai_);
        }

        ResolveResult(const ResolveResult& rr)
            : RefCount(rr),
              ai_(rr.ai_)
        {
        }

        ResolveResult& operator = (const ResolveResult& rr)
        {
            ResolveResult(rr).Swap(*this);
            return *this;
        }

        void Swap(ResolveResult& rr)
        {
            RefCount::Swap(rr);
            std::swap(rr.ai_, ai_);
        }

        iterator begin() const
        {
            return iterator(ai_);
        }

        iterator end() const
        {
            return iterator();
        }

    private:
        addrinfo *ai_;
    };

    class ResolveHint
    {
    public:
        ResolveHint()
        {
            memset(&hint_, 0, sizeof(hint_));
        }

        ResolveHint(int ai_family, int ai_socktype, int ai_protocol)
        {
            memset(&hint_, 0, sizeof(hint_));
            hint_.ai_family = ai_family;
            hint_.ai_socktype = ai_socktype;
            hint_.ai_protocol = ai_protocol;
        }

        const addrinfo * Get() const
        {
            return &hint_;
        }

    private:
        addrinfo hint_;
    };

    class AddressResolveException
    {
    public:
        AddressResolveException(int ec)
            : ec_(ec)
        {
        }

        // return value is one of this:
        // EAI_AGAIN, EAI_BADFLAGS, EAI_FAIL, EAI_FAMILY,
        // EAI_MEMORY, EAI_NONAME, EAI_SERVICE, EAI_SOCKTYPE
        int GetEAICode() const
        {
            return ec_;
        }

    private:
        int ec_;
    };

    inline ResolveResult ResolveAddress(const std::string& nodename,
                                        const std::string& servname,
                                        const ResolveHint& hint)
    {
        addrinfo *result = 0;
        int ec = ::getaddrinfo(nodename.c_str(), servname.c_str(), hint.Get(), &result);
        if (ec)
            throw AddressResolveException(ec);

        return ResolveResult(result);
    }

} // namespace net
} // namespace bitwave

#endif // ADDRESS_RESOLVER_H

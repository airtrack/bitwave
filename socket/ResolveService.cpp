#include "ResolveService.h"
#include "../base/ScopePtr.h"
#include "../thread/Thread.h"

namespace bittorrent
{
    struct AsyncResolveHelper
    {
        AsyncResolveHelper(ResolveService& serv,
                           const std::string& n,
                           const std::string& s,
                           const ResolveHint& resolvehint,
                           AsyncResolveHandler resolvehandler)
            : service(serv),
              nodename(n),
              servname(s),
              hint(resolvehint),
              handler(resolvehandler)
        {
        }

        ResolveService& service;
        std::string nodename;
        std::string servname;
        ResolveHint hint;
        AsyncResolveHandler handler;
    };

    // static
    unsigned __stdcall ResolveService::ResolveThread(void *arg)
    {
        ScopePtr<AsyncResolveHelper> helper(
                static_cast<AsyncResolveHelper *>(arg));

        try
        {
            ResolveResult result = ResolveAddress(helper->nodename,
                    helper->servname, helper->hint);

            // add result to ResolveService
            helper->service.AddResolveAddress(helper->nodename, result, helper->handler);
        }
        catch (const AddressResolveException&)
        {
            // we do nothing here
        }

        return 0;
    }

    void ResolveService::Run()
    {
        CallbackResolvedAddress();
    }

    void ResolveService::AsyncResolve(const std::string& nodename,
                                      const std::string& servname,
                                      const ResolveHint& hint,
                                      AsyncResolveHandler handler)
    {
        Thread resolver(ResolveThread, new AsyncResolveHelper(
                    *this, nodename, servname, hint, handler));
    }

    void ResolveService::AddResolveAddress(const std::string& nodename,
                                           ResolveResult result,
                                           AsyncResolveHandler handler)
    {
        SpinlocksMutexLocker lock(resolvedaddrmutex_);
        resolvedaddress_.insert(
            std::make_pair(nodename, AddressResolveValue(result, handler)));
    }

    void ResolveService::CallbackResolvedAddress()
    {
        ResolvedAddress address;
        {
            SpinlocksMutexLocker lock(resolvedaddrmutex_);
            address.swap(resolvedaddress_);
        }

        for (ResolvedAddress::iterator it = address.begin();
                it != address.end(); ++it)
        {
            it->second.second(it->first, it->second.first);
        }
    }
} // namespace bittorrent

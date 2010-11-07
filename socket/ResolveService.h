#ifndef RESOLVE_SERVICE_H
#define RESOLVE_SERVICE_H

#include "AddressResolver.h"
#include "../base/BaseTypes.h"
#include "../thread/Mutex.h"

#include <map>
#include <string>

namespace bittorrent
{
    // param are nodename and its ResolveResult
    typedef void (*AsyncResolveHandler)(const std::string&, ResolveResult);

    class ResolveService : private NotCopyable
    {
    public:
        void Run();
        void AsyncResolve(const std::string& nodename,
                          const std::string& servname,
                          const ResolveHint& hint,
                          AsyncResolveHandler handler);

    private:
        static unsigned __stdcall ResolveThread(void *arg);

        void AddResolveAddress(const std::string& nodename,
                               ResolveResult result,
                               AsyncResolveHandler handler);
        void CallbackResolvedAddress();

        typedef std::pair<ResolveResult, AsyncResolveHandler> AddressResolveValue;
        typedef std::map<std::string, AddressResolveValue> ResolvedAddress;
        ResolvedAddress resolvedaddress_;
        SpinlocksMutex resolvedaddrmutex_;
    };
} // namespace bittorrent

#endif // RESOLVE_SERVICE_H

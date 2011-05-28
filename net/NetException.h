#ifndef NET_EXCEPTION_H
#define NET_EXCEPTION_H

#include "../base/BaseTypes.h"

namespace bitwave {
namespace net {

    // net exception code
    enum NetExceptionCode
    {
        CREATE_SOCKET_ERROR,
        CREATE_SERVICE_ERROR,
        REGISTER_SOCKET_ERROR,
        BIND_LISTENER_SOCKET_ERROR,
        LISTEN_LISTENER_SOCKET_ERROR,
        GET_ACCEPTEX_FUNCTION_ERROR,
        GET_CONNECTEX_FUNCTION_ERROR,
        CALL_ACCEPTEX_FUNCTION_ERROR,
        CALL_CONNECTEX_FUNCTION_ERROR,
        CALL_WSARECV_FUNCTION_ERROR,
        CALL_WSASEND_FUNCTION_ERROR,
        CONNECT_BIND_LOCAL_ERROR,
    };

    // an exception class for net
    class NetException : public BaseException
    {
    public:
        explicit NetException(int code)
            : BaseException(code)
        {
        }
    };

    // exception code:
    // EAI_AGAIN, EAI_BADFLAGS, EAI_FAIL, EAI_FAMILY,
    // EAI_MEMORY, EAI_NONAME, EAI_SERVICE, EAI_SOCKTYPE
    class AddressResolveException : public BaseException
    {
    public:
        AddressResolveException(int code, const std::string& w)
            : BaseException(code, w)
        {
        }
    };

} // namespace net
} // namespace bitwave

#endif // NET_EXCEPTION_H

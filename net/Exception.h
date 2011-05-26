#ifndef EXCEPTION_H
#define EXCEPTION_H

namespace bitwave {
namespace net {

    // net exception error code
    enum
    {
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
    class NetException
    {
    public:
        explicit NetException(int code)
            : code_(code)
        {
        }

        int GetCode() const
        {
            return code_;
        }

    private:
        int code_;
    };

} // namespace net
} // namespace bitwave

#endif // EXCEPTION_H

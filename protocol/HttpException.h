#ifndef HTTP_EXCEPTION_H
#define HTTP_EXCEPTION_H

#include "../base/BaseTypes.h"

namespace bitwave {
namespace http {

    enum URIExceptionCode
    {
        NO_SCHEME,
        NO_AUTHORITY,
    };

    // an exception class for URI
    class URIException : public BaseException
    {
    public:
        explicit URIException(int code)
            : BaseException(code)
        {
        }
    };

    // an exception class of construct Response
    class ResponseException : public BaseException
    {
    public:
        explicit ResponseException(const std::string& w)
            : BaseException(w)
        {
        }
    };

} // namespace http
} // namespace bitwave

#endif // HTTP_EXCEPTION_H

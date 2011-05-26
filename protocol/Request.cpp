#include "Request.h"

namespace bitwave {
namespace http {

    Request::Request(const URI& uri)
        : text_()
    {
        text_.append("GET ");
        text_.append(uri.GetQueryString());
        text_.append(" HTTP/1.1\r\n");
        text_.append("Host: ");
        text_.append(uri.GetHost());
        text_.append("\r\n\r\n");
    }

} // namespace http
} // namespace bitwave

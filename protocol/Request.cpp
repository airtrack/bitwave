#include "Request.h"

namespace bittorrent
{
namespace http
{
    Request::Request(const URI& uri)
        : text_()
    {
        text_.append("GET ");
        text_.append(uri.GetURIString());
        text_.append(" HTTP/1.1\r\n\r\n");
    }
} // namespace http
} // namespace bittorrent

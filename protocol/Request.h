#ifndef REQUEST_H
#define REQUEST_H

#include "URI.h"
#include <string>

namespace bittorrent {
namespace http {

    // this is a simply http GET request just hold an URI.
    class Request
    {
    public:
        explicit Request(const URI& uri);

        std::string GetRequestText() const
        {
            return text_;
        }

    private:
        std::string text_;
    };

} // namespace http
} // namespace bittorrent

#endif // REQUEST_H

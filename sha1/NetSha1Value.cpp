#include "NetSha1Value.h"
#include "Sha1Value.h"
#include "../net/NetHelper.h"
#include <string.h>

namespace bittorrent {

    Sha1Value NetByteOrder(const Sha1Value& value)
    {
        Sha1Value result(value);
        unsigned *v = reinterpret_cast<unsigned *>(
                const_cast<char *>(result.GetData())
                );
        net::HostToNeti(v, result.GetDataSize() / sizeof(unsigned));

        return result;
    }

    Sha1Value NetStreamToSha1Value(const char *stream)
    {
        Sha1Value result;
        unsigned *v = reinterpret_cast<unsigned *>(
                const_cast<char *>(result.GetData())
                );
        memcpy(v, stream, result.GetDataSize());
        net::NetToHosti(v, result.GetDataSize() / sizeof(unsigned));

        return result;
    }

} // namespace bittorrent

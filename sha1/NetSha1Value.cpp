#include "NetSha1Value.h"
#include "Sha1Value.h"
#include "../net/NetHelper.h"

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

} // namespace bittorrent

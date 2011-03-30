#ifndef NET_SHA1_VALUE_H
#define NET_SHA1_VALUE_H

namespace bittorrent {

    class Sha1Value;

    Sha1Value NetByteOrder(const Sha1Value& value);
    Sha1Value NetStreamToSha1Value(const char *stream);

} // namespace bittorrent

#endif // NET_SHA1_VALUE_H

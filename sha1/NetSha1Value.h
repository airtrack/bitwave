#ifndef NET_SHA1_VALUE_H
#define NET_SHA1_VALUE_H

namespace bitwave {

    class Sha1Value;

    Sha1Value NetByteOrder(const Sha1Value& value);
    Sha1Value NetStreamToSha1Value(const char *stream);

} // namespace bitwave

#endif // NET_SHA1_VALUE_H

#ifndef BIT_PEER_CREATE_STRATEGY
#define BIT_PEER_CREATE_STRATEGY

#include <cstddef>

namespace bitwave {
namespace core {

    // base class of create peer strategy for BitTask
    class BitPeerCreateStrategy
    {
    public:
        // return next create peer interval, in millisecond
        virtual std::size_t CreatePeerInterval(std::size_t current_peer_count) = 0;
        // return need create new peers count
        virtual std::size_t CreatePeerCount(std::size_t current_peer_count,
                                            std::size_t unused_listen_info) = 0;
        virtual ~BitPeerCreateStrategy() {}
    };

    BitPeerCreateStrategy * CreateDefaultPeerCreateStartegy();

} // namespace core
} // namespace bitwave

#endif // BIT_PEER_CREATE_STRATEGY

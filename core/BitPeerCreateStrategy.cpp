#include "BitPeerCreateStrategy.h"

namespace bittorrent {
namespace core {

    namespace interval {

        class NormalPeerCreateStrategy : public BitPeerCreateStrategy
        {
        public:
            explicit NormalPeerCreateStrategy(std::size_t min_peer_count)
                : min_peer_count_(min_peer_count)
            {
            }

            virtual std::size_t CreatePeerInterval(std::size_t current_peer_count)
            {
                if (current_peer_count < min_peer_count_ / 2)
                    return 1000;
                else if(current_peer_count < min_peer_count_)
                    return 5000;
                else
                    return 30000;
            }

            virtual std::size_t CreatePeerCount(std::size_t current_peer_count,
                                                std::size_t unused_listen_info)
            {
                // total peers must greater then min_peer_count_ at least
                if (current_peer_count < min_peer_count_)
                    return unused_listen_info;

                // we create new peers appropriately
                if (unused_listen_info <= min_peer_count_ / 10)
                    return unused_listen_info;
                else if (unused_listen_info <= min_peer_count_ / 5)
                    return unused_listen_info / 2;
                else if (unused_listen_info <= min_peer_count_ / 2)
                    return unused_listen_info / 5;
                else
                    return unused_listen_info / 10;
            }

        private:
            std::size_t min_peer_count_;
        };

    } // namespace interval

    BitPeerCreateStrategy * CreateDefaultPeerCreateStartegy()
    {
        return new interval::NormalPeerCreateStrategy(200);
    }

} // namespace core
} // namespace bittorrent

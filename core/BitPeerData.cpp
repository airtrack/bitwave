#include "BitPeerData.h"

namespace bittorrent {
namespace core {

    BitPeerData::BitPeerData(const std::string& peer_id,
                             std::size_t piece_count)
        : peer_id_(peer_id),
          piece_map_(piece_count)
    {
    }

} // namespace core
} // namespace bittorrent

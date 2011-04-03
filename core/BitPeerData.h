#ifndef BIT_PEER_DATA_H
#define BIT_PEER_DATA_H

#include "BitPieceMap.h"
#include "../base/BaseTypes.h"
#include <string>

namespace bittorrent {
namespace core {

    class BitPeerData : private NotCopyable
    {
    public:
        explicit BitPeerData(const std::string& peer_id,
                             std::size_t piece_count);

    private:
        std::string peer_id_;
        BitPieceMap piece_map_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_PEER_DATA_H

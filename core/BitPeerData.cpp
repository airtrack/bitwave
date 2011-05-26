#include "BitPeerData.h"

namespace bitwave {
namespace core {

    BitPeerData::BitPeerData(const std::string& peer_id,
                             std::size_t piece_count)
        : peer_id_(peer_id),
          piece_map_(piece_count)
    {
    }

    void BitPeerData::PeerHavePiece(int piece_index)
    {
        piece_map_.MarkPiece(piece_index);
    }

    bool BitPeerData::SetPeerBitfield(const char *bit_field, std::size_t size)
    {
        return piece_map_.MarkPieceFromBitfield(bit_field, size);
    }

    const BitPieceMap& BitPeerData::GetPieceMap() const
    {
        return piece_map_;
    }

} // namespace core
} // namespace bitwave

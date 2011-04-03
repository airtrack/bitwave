#ifndef BIT_PIECE_MAP_H
#define BIT_PIECE_MAP_H

#include "../base/RefCount.h"

namespace bittorrent {
namespace core {

    class BitPieceMap : public RefCount
    {
    public:
        explicit BitPieceMap(std::size_t piece_count);

        BitPieceMap(const char *bit_field, std::size_t len);

        BitPieceMap(const BitPieceMap& piece_map);

        ~BitPieceMap();

        BitPieceMap& operator = (const BitPieceMap& piece_map);

        void Swap(BitPieceMap& piece_map);

        void MarkPiece(std::size_t piece_index);

        // return piece_map1 difference piece_map2
        static BitPieceMap Difference(const BitPieceMap& piece_map1,
                                      const BitPieceMap& piece_map2);

    private:
        void InitMap();

        char *piece_map_;
        std::size_t map_size_;
    };

    // an exception class for BitPieceMap::Difference function
    class PieceMapSizeNotMatch { };

} // namespace core
} // namespace bittorrent

#endif // BIT_PIECE_MAP_H

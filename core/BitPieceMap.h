#ifndef BIT_PIECE_MAP_H
#define BIT_PIECE_MAP_H

#include "../base/RefCount.h"

namespace bittorrent {
namespace core {

    class BitPieceMap : public RefCount
    {
    public:
        explicit BitPieceMap(std::size_t piece_count);

        BitPieceMap(const BitPieceMap& piece_map);

        ~BitPieceMap();

        BitPieceMap& operator = (const BitPieceMap& piece_map);

        void Swap(BitPieceMap& piece_map);

        void MarkPiece(std::size_t piece_index);

        bool MarkPieceFromBitfield(const char *bit_field, std::size_t size);

        std::size_t GetMapSize() const;

        bool IsPieceMark(std::size_t piece_index) const;

        // store to bit_field, size must bigger than result of GetMapSize()
        void ToBitfield(char *bit_field) const;

        static void Difference(const BitPieceMap& piece_map1,
                               const BitPieceMap& piece_map2,
                               std::size_t begin, std::size_t end,
                               BitPieceMap& result);

        static void Intersection(const BitPieceMap& piece_map1,
                                 const BitPieceMap& piece_map2,
                                 std::size_t begin, std::size_t end,
                                 BitPieceMap& result);

        static void Union(const BitPieceMap& piece_map1,
                          const BitPieceMap& piece_map2,
                          std::size_t begin, std::size_t end,
                          BitPieceMap& result);

        static bool IsEqual(const BitPieceMap& piece_map1,
                            const BitPieceMap& piece_map2,
                            std::size_t begin, std::size_t end);

    private:
        void InitMap();

        char *piece_map_;
        std::size_t map_size_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_PIECE_MAP_H

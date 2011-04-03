#include "BitPieceMap.h"
#include <assert.h>
#include <math.h>
#include <string.h>

namespace bittorrent {
namespace core {

    BitPieceMap::BitPieceMap(std::size_t piece_count)
        : RefCount(true),
          piece_map_(0),
          map_size_(0)
    {
        assert(piece_count > 0);
        map_size_ = static_cast<std::size_t>(
            ceil(static_cast<double>(piece_count) / 8));
        InitMap();
    }

    BitPieceMap::BitPieceMap(const char *bit_field, std::size_t len)
        : RefCount(true),
          piece_map_(0),
          map_size_(len)
    {
        assert(len > 0);
        InitMap();
        memcpy(piece_map_, bit_field, map_size_);
    }

    BitPieceMap::BitPieceMap(const BitPieceMap& piece_map)
        : RefCount(piece_map),
          piece_map_(piece_map.piece_map_),
          map_size_(piece_map.map_size_)
    {
    }

    BitPieceMap::~BitPieceMap()
    {
        if (Only())
            delete [] piece_map_;
    }

    BitPieceMap& BitPieceMap::operator = (const BitPieceMap& piece_map)
    {
        BitPieceMap(piece_map).Swap(*this);
        return *this;
    }

    void BitPieceMap::Swap(BitPieceMap& piece_map)
    {
        RefCount::Swap(piece_map);
        std::swap(piece_map_, piece_map.piece_map_);
        std::swap(map_size_, piece_map.map_size_);
    }

    void BitPieceMap::MarkPiece(std::size_t piece_index)
    {
        std::size_t index = piece_index / 8;
        std::size_t bit_index = piece_index - 8 * index;
        if (index < map_size_)
            piece_map_[index] |= 0x01 << (7 - bit_index);
    }

    void BitPieceMap::InitMap()
    {
        assert(!piece_map_);
        piece_map_ = new char[map_size_];
        memset(piece_map_, 0, map_size_);
    }

    // static
    BitPieceMap BitPieceMap::Difference(const BitPieceMap& piece_map1,
                                        const BitPieceMap& piece_map2)
    {
        assert(piece_map1.piece_map_);
        assert(piece_map2.piece_map_);

        if (piece_map1.map_size_ != piece_map2.map_size_)
            throw PieceMapSizeNotMatch();

        BitPieceMap result(piece_map1.map_size_ * 8);
        assert(result.map_size_ == piece_map1.map_size_);

        for (std::size_t i = 0; i < result.map_size_; ++i)
        {
            result.piece_map_[i] =
                piece_map1.piece_map_[i] & (~piece_map2.piece_map_[i]);
        }

        return result;
    }

} // namespace core
} // namespace bittorrent

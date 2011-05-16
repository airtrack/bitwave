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

    void BitPieceMap::Clear()
    {
        memset(piece_map_, 0, map_size_);
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

    void BitPieceMap::UnMarkPiece(std::size_t piece_index)
    {
        std::size_t index = piece_index / 8;
        std::size_t bit_index = piece_index - 8 * index;
        if (index < map_size_)
            piece_map_[index] &= ~(0x01 << (7 - bit_index));
    }

    bool BitPieceMap::MarkPieceFromBitfield(const char *bit_field, std::size_t size)
    {
        if (map_size_ != size)
            return false;
        memcpy(piece_map_, bit_field, map_size_);
        return true;
    }

    std::size_t BitPieceMap::GetMapSize() const
    {
        return map_size_;
    }

    bool BitPieceMap::IsPieceMark(std::size_t piece_index) const
    {
        std::size_t index = piece_index / 8;
        std::size_t bit_index = piece_index - 8 * index;
        if (index < map_size_)
            return (piece_map_[index] & (0x01 << (7 - bit_index))) != 0;
        return false;
    }

    void BitPieceMap::ToBitfield(char *bit_field) const
    {
        assert(bit_field);
        memcpy(bit_field, piece_map_, map_size_);
    }

    void BitPieceMap::InitMap()
    {
        assert(!piece_map_);
        piece_map_ = new char[map_size_];
        Clear();
    }

    // static
    void BitPieceMap::Difference(const BitPieceMap& piece_map1,
                                 const BitPieceMap& piece_map2,
                                 std::size_t begin, std::size_t end,
                                 BitPieceMap& result)
    {
        assert(piece_map1.piece_map_);
        assert(piece_map2.piece_map_);
        assert(result.piece_map_);
        assert(result.map_size_ == piece_map1.map_size_);
        assert(result.map_size_ == piece_map2.map_size_);
        assert(begin <= result.map_size_ && end <= result.map_size_);

        for (; begin < end; ++begin)
        {
            result.piece_map_[begin] =
                piece_map1.piece_map_[begin] & (~piece_map2.piece_map_[begin]);
        }
    }

    // static
    void BitPieceMap::Intersection(const BitPieceMap& piece_map1,
                                   const BitPieceMap& piece_map2,
                                   std::size_t begin, std::size_t end,
                                   BitPieceMap& result)
    {
        assert(piece_map1.piece_map_);
        assert(piece_map2.piece_map_);
        assert(result.piece_map_);
        assert(result.map_size_ == piece_map1.map_size_);
        assert(result.map_size_ == piece_map2.map_size_);
        assert(begin <= result.map_size_ && end <= result.map_size_);

        for (; begin < end; ++begin)
        {
            result.piece_map_[begin] =
                piece_map1.piece_map_[begin] & piece_map2.piece_map_[begin];
        }
    }

    // static
    void BitPieceMap::Union(const BitPieceMap& piece_map1,
                            const BitPieceMap& piece_map2,
                            std::size_t begin, std::size_t end,
                            BitPieceMap& result)
    {
        assert(piece_map1.piece_map_);
        assert(piece_map2.piece_map_);
        assert(result.piece_map_);
        assert(result.map_size_ == piece_map1.map_size_);
        assert(result.map_size_ == piece_map2.map_size_);
        assert(begin <= result.map_size_ && end <= result.map_size_);

        for (; begin < end; ++begin)
        {
            result.piece_map_[begin] =
                piece_map1.piece_map_[begin] | piece_map2.piece_map_[begin];
        }
    }

    bool BitPieceMap::IsEqual(const BitPieceMap& piece_map1,
                              const BitPieceMap& piece_map2,
                              std::size_t begin, std::size_t end)
    {
        assert(piece_map1.piece_map_);
        assert(piece_map2.piece_map_);
        assert(piece_map1.map_size_ == piece_map2.map_size_);
        assert(begin <= piece_map1.map_size_&& end <= piece_map1.map_size_);
        assert(begin <= end);

        std::size_t size = end - begin;
        if (size == 0)
            return false;

        return memcmp(piece_map1.piece_map_ + begin,
                      piece_map2.piece_map_ + begin, size) == 0;
    }

} // namespace core
} // namespace bittorrent

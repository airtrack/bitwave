#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include "BitCache.h"
#include "BitData.h"
#include "BitPiece.h"
#include "BitPieceMap.h"
#include "BitService.h"
#include "BitController.h"
#include "BitDownloadDispatcher.h"
#include "bencode/MetainfoFile.h"
#include <assert.h>
#include <vector>

namespace bittorrent {
namespace core {

    BitCache::BitCache(const std::tr1::shared_ptr<BitData>& bitdata)
        : piece_length_(bitdata->GetPieceLength()),
          piece_map_(bitdata->GetPieceMap()),
          info_hash_(bitdata->GetInfoHash()),
          metainfo_file_(bitdata->GetMetainfoFile()),
          file_(bitdata)
    {
        assert(BitService::controller);
        download_dispatcher_ =
            BitService::controller->GetTaskDownloadDispather(info_hash_);
        assert(download_dispatcher_);
    }

    void BitCache::Read(std::size_t piece_index,
                        std::size_t begin_of_piece,
                        std::size_t length,
                        const ReadCallback& callback)
    {
        if (!piece_map_.IsPieceMark(piece_index) || begin_of_piece + length > piece_length_)
            callback(false, 0);
        else
            ReadBlock(piece_index, begin_of_piece, length, callback);
    }

    void BitCache::Write(std::size_t piece_index,
                         std::size_t begin_of_piece,
                         std::size_t length,
                         const char *block)
    {
        if (piece_map_.IsPieceMark(piece_index))
            return ;

        WriteBlock(piece_index, begin_of_piece, length, block);
    }

    void BitCache::ProcessCache()
    {
        ProcessAsyncReadOps();
        ProcessAsyncCheckPiece();
        ProcessAsyncWritePiece();
    }

    BitCache::PiecePtr BitCache::FetchNewPiece()
    {
        return PiecePtr(new BitPiece(piece_length_));
    }

    BitCache::CachePiece::iterator BitCache::InsertNewPiece(std::size_t piece_index)
    {
        PiecePtr piece = FetchNewPiece();
        return cache_piece_.insert(cache_piece_.begin(),
                std::make_pair(piece_index, piece));
    }

    void BitCache::ReadBlock(std::size_t piece_index,
                             std::size_t begin_of_piece,
                             std::size_t length,
                             const ReadCallback& callback)
    {
        CachePiece::iterator it = cache_piece_.find(piece_index);
        if (it == cache_piece_.end())
            AsyncReadBlock(piece_index, begin_of_piece, length, callback);
        else
            ReadCacheBlock(it, begin_of_piece, length, callback);
    }

    void BitCache::AsyncReadBlock(std::size_t piece_index,
                                  std::size_t begin_of_piece,
                                  std::size_t length,
                                  const ReadCallback& callback)
    {
        AsyncReadOps::iterator it = async_read_ops_.find(piece_index);
        if (it == async_read_ops_.end())
            file_.ReadPiece(piece_index, FetchNewPiece());

        async_read_ops_.insert(it,
                std::make_pair(piece_index,
                    AsyncReadData(callback, begin_of_piece, length)));
    }

    void BitCache::ReadCacheBlock(CachePiece::iterator it,
                                  std::size_t begin_of_piece,
                                  std::size_t length,
                                  const ReadCallback& callback)
    {
        const char *block = it->second->GetRawDataPtr() + begin_of_piece;
        callback(true, block);
    }

    void BitCache::ProcessAsyncReadOps()
    {
        CachePiece read_pieces;
        file_.GetReadPieces(read_pieces);

        for (CachePiece::iterator it = read_pieces.begin();
                it != read_pieces.end(); ++it)
            CompleteAsyncReadOps(it);

        cache_piece_.insert(read_pieces.begin(), read_pieces.end());
    }

    void BitCache::CompleteAsyncReadOps(CachePiece::iterator it)
    {
        std::pair<AsyncReadOps::iterator, AsyncReadOps::iterator> range =
            async_read_ops_.equal_range(it->first);

        for (; range.first != range.second; ++range.first)
        {
            AsyncReadData& data = range.first->second;
            ReadCacheBlock(it, data.begin_of_piece, data.length, data.callback);
        }

        async_read_ops_.erase(it->first);
    }

    void BitCache::WriteBlock(std::size_t piece_index,
                              std::size_t begin_of_piece,
                              std::size_t length,
                              const char *block)
    {
        CachePiece::iterator it = cache_piece_.find(piece_index);
        if (it == cache_piece_.end())
            it = InsertNewPiece(piece_index);

        // this piece is CHECKING_SHA1 CHECK_SHA1_OK or WRITED,
        // we do not change this piece's data
        if (it->second->GetState() != BitPiece::NOT_CHECKED)
            return ;

        it->second->WriteBlock(begin_of_piece, length, block);
        if (it->second->IsComplete())
            AsyncCheckPiece(it);
    }

    void BitCache::AsyncCheckPiece(CachePiece::iterator it)
    {
        it->second->SetState(BitPiece::CHECKING_SHA1);
        piece_sha1_calc_.AddPiece(it->first, it->second);
    }

    void BitCache::ProcessAsyncCheckPiece()
    {
        BitPieceSha1Calc::PieceSha1List sha1_list;
        piece_sha1_calc_.GetResult(sha1_list);

        BitPieceSha1Calc::PieceSha1List::iterator it = sha1_list.begin();
        for (; it != sha1_list.end(); ++it)
            CompleteAsyncCheckPiece(it->first, it->second);
    }

    void BitCache::CompleteAsyncCheckPiece(std::size_t piece_index,
                                           const Sha1Value& sha1)
    {
        CachePiece::iterator it = cache_piece_.find(piece_index);
        if (it == cache_piece_.end())
            return ;

        if (sha1 == metainfo_file_->Pieces(piece_index))
        {
            it->second->SetState(BitPiece::CHECK_SHA1_OK);
            AsyncWritePiece(it);
        }
        else
        {
            it->second->SetState(BitPiece::NOT_CHECKED);
            download_dispatcher_->ReDownloadPiece(piece_index);
        }
    }

    void BitCache::AsyncWritePiece(CachePiece::iterator it)
    {
        file_.WritePiece(it->first, it->second);
    }

    void BitCache::ProcessAsyncWritePiece()
    {
        std::vector<std::size_t> writed_pieces;
        file_.GetWritedPieces(writed_pieces);

        std::vector<std::size_t>::iterator it = writed_pieces.begin();
        for (; it != writed_pieces.end(); ++it)
            CompleteAsyncWritePiece(*it);
    }

    void BitCache::CompleteAsyncWritePiece(std::size_t piece_index)
    {
        CachePiece::iterator it = cache_piece_.find(piece_index);
        if (it != cache_piece_.end())
            it->second->SetState(BitPiece::WRITED);

        download_dispatcher_->CompletePiece(piece_index);
    }

} // namespace core
} // namespace bittorrent

#include "BitDownloadDispatcher.h"
#include "BitData.h"
#include "BitPeerData.h"
#include "BitService.h"
#include "BitController.h"
#include <assert.h>
#include <math.h>

namespace bitwave {
namespace core {

    // 16KB size of one request
    const int request_block_size = 16 * 1024;

    class LinearPieceIndexSearcher :
        public BitDownloadDispatcher::PieceIndexSearcher
    {
    public:
        explicit LinearPieceIndexSearcher(std::size_t pieces_count)
            : result_piece_map_(pieces_count),
              empty_piece_map_(pieces_count)
        {
        }

        virtual bool Search(const BitPieceMap& downloaded,
                            const BitPieceMap& downloading,
                            const BitPieceMap& need_download,
                            const BitPieceMap& candidate,
                            std::size_t *piece_index)
        {
            std::size_t size = downloaded.GetMapSize();
            std::size_t begin = 0;
            std::size_t end = size;
            while (begin < end)
            {
                std::size_t middle = (begin + end) / 2;
                middle = middle == begin ? begin + 1 : middle;

                BitPieceMap::Union(downloaded, downloading,
                        begin, middle, result_piece_map_);
                BitPieceMap::Difference(need_download, result_piece_map_,
                        begin, middle, result_piece_map_);
                BitPieceMap::Intersection(result_piece_map_, candidate,
                        begin, middle, result_piece_map_);
                bool is_empty = BitPieceMap::IsEqual(
                        result_piece_map_, empty_piece_map_, begin, middle);

                if (is_empty)
                    begin = middle;
                else
                    end = end == middle ? middle - 1 : middle;
            }

            assert(begin == end);
            if (begin != size)
                return SearchPieceIndex(begin, piece_index);
            return false;
        }

    private:
        bool SearchPieceIndex(std::size_t index, std::size_t *piece_index)
        {
            assert(piece_index);
            std::size_t begin_piece_index = index * 8;
            std::size_t end_piece_index = begin_piece_index + 8;

            while (begin_piece_index < end_piece_index &&
                   !result_piece_map_.IsPieceMark(begin_piece_index))
                ++begin_piece_index;

            if (begin_piece_index != end_piece_index)
            {
                *piece_index = begin_piece_index;
                return true;
            }

            return false;
        }

        BitPieceMap result_piece_map_;
        BitPieceMap empty_piece_map_;
    };

    BitDownloadDispatcher::BitDownloadDispatcher(
            const std::tr1::shared_ptr<BitData>& bitdata)
        : bitdata_(bitdata),
          pieces_count_(bitdata->GetPieceCount()),
          need_download_(bitdata->GetPieceCount()),
          downloading_(bitdata->GetPieceCount()),
          end_downloading_mode_(false)
    {
        std::size_t piece_length = bitdata->GetPieceLength();
        block_count_ = piece_length / request_block_size;

        PieceIndexSearcher *pis = new LinearPieceIndexSearcher(pieces_count_);
        piece_index_searcher_.Reset(pis);

        UpdateNeedDownload();
    }

    void BitDownloadDispatcher::DispatchRequestList(
            const std::tr1::shared_ptr<BitPeerData>& peer_data,
            BitRequestList& request_list)
    {
        // dispatch requests from the scattered_request_ first
        if (!scattered_request_.Empty())
            DispatchScatteredRequest(peer_data, request_list);

        if (request_list.Size() < block_count_)
            DispatchNewRequest(peer_data, request_list);
    }

    void BitDownloadDispatcher::ReturnRequest(BitRequestList& request_list,
                                              BitRequestList::Iterator it)
    {
        if (end_downloading_mode_ &&
            scattered_request_.IsExistRequest(it->index, it->begin, it->length))
        {
            // in end mode and scattered_request_ have the request,
            // we just Erase it
            request_list.Erase(it);
        }
        else
        {
            scattered_request_.Splice(request_list, it);
        }

        if (end_downloading_mode_)
            BitService::controller->LetAllPeerRequestPiece(bitdata_->GetInfoHash());
    }

    void BitDownloadDispatcher::ReDownloadPiece(std::size_t piece_index)
    {
        downloading_.UnMarkPiece(piece_index);

        if (end_downloading_mode_)
            BitService::controller->LetAllPeerRequestPiece(bitdata_->GetInfoHash());
    }

    void BitDownloadDispatcher::CompletePiece(std::size_t piece_index)
    {
        if (bitdata_->GetPieceMap().IsPieceMark(piece_index))
            return ;

        bitdata_->IncreaseDownloaded(bitdata_->GetPieceLength());
        bitdata_->GetPieceMap().MarkPiece(piece_index);
        downloading_.UnMarkPiece(piece_index);

        if (end_downloading_mode_)
            DeleteScatteredRequest(piece_index);

        BitService::controller->CompletePiece(
                bitdata_->GetInfoHash(), piece_index);

        if (bitdata_->IsDownloadComplete())
            CompleteDownload();
        else
            TryEnterEndMode();
    }

    void BitDownloadDispatcher::UpdateNeedDownload()
    {
        const BitData::DownloadFiles& files = bitdata_->GetFilesInfo();
        BitData::DownloadFiles::const_iterator it = files.begin();
        BitData::DownloadFiles::const_iterator end = files.end();

        long long file_begin = 0;
        std::size_t piece_length = bitdata_->GetPieceLength();
        for (; it != end; ++it)
        {
            long long file_end = file_begin + it->length;
            if (it->is_download)
            {
                std::size_t piece_index = static_cast<std::size_t>(
                        file_begin / piece_length);
                std::size_t end_piece_index = static_cast<std::size_t>(
                        ceil(file_end / static_cast<double>(piece_length)));

                for (; piece_index < end_piece_index; ++piece_index)
                    need_download_.MarkPiece(piece_index);
            }
            file_begin = file_end;
        }
    }

    void BitDownloadDispatcher::DispatchScatteredRequest(
            const std::tr1::shared_ptr<BitPeerData>& peer_data,
            BitRequestList& request_list)
    {
        const BitPieceMap& peer_piece_map = peer_data->GetPieceMap();
        BitRequestList::Iterator it = scattered_request_.Begin();

        std::size_t count = 0;
        while (count < block_count_ && it != scattered_request_.End())
        {
            if (peer_piece_map.IsPieceMark(it->index))
            {
                if (end_downloading_mode_)
                {
                    // in end mode, do not Splice the request to request_list
                    request_list.AddRequest(it->index, it->begin, it->length);
                    ++it;
                }
                else
                {
                    request_list.Splice(scattered_request_, it++);
                }
                ++count;
            }
            else
            {
                ++it;
            }
        }
    }

    void BitDownloadDispatcher::DispatchNewRequest(
            const std::tr1::shared_ptr<BitPeerData>& peer_data,
            BitRequestList& request_list)
    {
        const BitPieceMap& downloaded_map = bitdata_->GetPieceMap();
        const BitPieceMap& peer_piece_map = peer_data->GetPieceMap();

        std::size_t piece_index = 0;
        bool is_search = piece_index_searcher_->Search(
                downloaded_map, downloading_, need_download_,
                peer_piece_map, &piece_index);

        if (is_search && piece_index < pieces_count_)
        {
            // in end mode, do not mark the piece, it will let all peers
            // request the piece
            if (!end_downloading_mode_)
                downloading_.MarkPiece(piece_index);

            for (std::size_t i = 0; i < block_count_; ++i)
                request_list.AddRequest(piece_index,
                        i * request_block_size, request_block_size);
        }
    }

    void BitDownloadDispatcher::DeleteScatteredRequest(std::size_t piece_index)
    {
        BitRequestList::Iterator it = scattered_request_.Begin();
        while (it != scattered_request_.End())
        {
            if (it->index == piece_index)
                scattered_request_.Erase(it++);
            else
                ++it;
        }
    }

    void BitDownloadDispatcher::TryEnterEndMode()
    {
        if (!end_downloading_mode_)
        {
            long long downloaded = bitdata_->GetDownloaded();
            long long total_size = bitdata_->GetTotalSize();
            long long left_size = total_size - downloaded;
            double left_percent =
                static_cast<double>(left_size) / static_cast<double>(total_size);
            if (left_percent <= 0.05 && left_size <= 30 * 1024 * 1024)
                EnterEndMode();
        }
    }

    void BitDownloadDispatcher::EnterEndMode()
    {
        end_downloading_mode_ = true;
        BitService::controller->LetAllPeerRequestPiece(bitdata_->GetInfoHash());
    }

    void BitDownloadDispatcher::CompleteDownload()
    {
        BitService::controller->CompleteDownload(bitdata_->GetInfoHash());
        end_downloading_mode_ = false;
        downloading_.Clear();
        scattered_request_.Clear();
    }

} // namespace core
} // namespace bitwave

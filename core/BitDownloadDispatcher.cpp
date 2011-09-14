#include "BitDownloadDispatcher.h"
#include "BitData.h"
#include "BitPeerData.h"
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
            const std::tr1::shared_ptr<BitData>& bitdata,
            BitDownloadingInfo *downloading_info)
        : bitdata_(bitdata),
          downloading_info_(downloading_info),
          pieces_count_(bitdata->GetPieceCount()),
          end_downloading_mode_(false)
    {
        std::size_t piece_length = bitdata->GetPieceLength();
        block_count_ = piece_length / request_block_size;

        PieceIndexSearcher *pis = new LinearPieceIndexSearcher(pieces_count_);
        piece_index_searcher_.Reset(pis);

        downloading_info_->AddInfoObserver(this);
    }

    BitDownloadDispatcher::~BitDownloadDispatcher()
    {
        downloading_info_->RemoveInfoObserver(this);
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
    }

    void BitDownloadDispatcher::CompleteNewPiece(std::size_t piece_index)
    {
        if (end_downloading_mode_)
            DeleteScatteredRequest(piece_index);
        else
            EnterEndDownloadMode();
    }

    void BitDownloadDispatcher::DispatchScatteredRequest(
            const std::tr1::shared_ptr<BitPeerData>& peer_data,
            BitRequestList& request_list)
    {
        const BitPieceMap& peer_piece_map = peer_data->GetPieceMap();
        BitRequestList::Iterator it = scattered_request_.Begin();

        std::size_t count = 0;
        BitRequestList dispatched;
        while (count < block_count_ && it != scattered_request_.End())
        {
            if (peer_piece_map.IsPieceMark(it->index))
            {
                dispatched.Splice(scattered_request_, it++);
                ++count;
            }
            else
            {
                ++it;
            }
        }

        if (end_downloading_mode_)
        {
            request_list.AddRequest(dispatched.Begin(), dispatched.End());
            scattered_request_.Splice(dispatched, dispatched.Begin(), dispatched.End());
        }
        else
        {
            request_list.Splice(dispatched, dispatched.Begin(), dispatched.End());
        }
    }

    void BitDownloadDispatcher::DispatchNewRequest(
            const std::tr1::shared_ptr<BitPeerData>& peer_data,
            BitRequestList& request_list)
    {
        std::size_t piece_index = 0;
        bool is_search = piece_index_searcher_->Search(
                downloading_info_->GetDownloaded(),
                downloading_info_->GetDownloading(),
                downloading_info_->GetNeedDownload(),
                peer_data->GetPieceMap(), &piece_index);

        if (is_search && piece_index < pieces_count_)
        {
            downloading_info_->MarkDownloading(piece_index);
            if (end_downloading_mode_)
            {
                ScatterRequestPiece(piece_index, scattered_request_);
                DispatchScatteredRequest(peer_data, request_list);
            }
            else
            {
                ScatterRequestPiece(piece_index, request_list);
            }
        }
    }

    void BitDownloadDispatcher::ScatterRequestPiece(
            std::size_t piece_index,
            BitRequestList& list)
    {
        for (std::size_t i = 0; i < block_count_; ++i)
        {
            list.AddRequest(piece_index,
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

    bool BitDownloadDispatcher::EnterEndDownloadMode()
    {
        long long downloaded = bitdata_->GetDownloaded();
        long long total_size = bitdata_->GetTotalSize();
        long long left_size = total_size - downloaded;

        double left_percent =
            static_cast<double>(left_size) / static_cast<double>(total_size);
        if (left_percent <= 0.05 && left_size <= 30 * 1024 * 1024)
            end_downloading_mode_ = true;

        return end_downloading_mode_;
    }

} // namespace core
} // namespace bitwave

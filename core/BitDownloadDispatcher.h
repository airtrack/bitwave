#ifndef BIT_DOWNLOAD_DISPATCHER_H
#define BIT_DOWNLOAD_DISPATCHER_H

#include "BitPieceMap.h"
#include "BitRequestList.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include <memory>

namespace bittorrent {
namespace core {

    class BitData;
    class BitPeerData;

    class BitDownloadDispatcher : private NotCopyable
    {
    public:
        class PieceIndexSearcher
        {
        public:
            virtual bool Search(const BitPieceMap& downloaded,
                                const BitPieceMap& downloading,
                                const BitPieceMap& need_download,
                                const BitPieceMap& candidate,
                                std::size_t *piece_index) = 0;
            virtual ~PieceIndexSearcher() { }
        };

        explicit BitDownloadDispatcher(
                const std::tr1::shared_ptr<BitData>& bitdata);

        void DispatchRequestList(
                const std::tr1::shared_ptr<BitPeerData>& peer_data,
                BitRequestList& request_list);

        void ReturnRequest(BitRequestList& request_list,
                           BitRequestList::Iterator it);

        void ReDownloadPiece(std::size_t piece_index);

        void CompletePiece(std::size_t piece_index);

        bool IsEndDownloadingMode() const
        {
            return end_downloading_mode_;
        }

    private:
        void UpdateNeedDownload();
        void DispatchScatteredRequest(
                const std::tr1::shared_ptr<BitPeerData>& peer_data,
                BitRequestList& request_list);
        void DispatchNewRequest(
                const std::tr1::shared_ptr<BitPeerData>& peer_data,
                BitRequestList& request_list);
        void DeleteScatteredRequest(std::size_t piece_index);
        void TryEnterEndMode();
        void EnterEndMode();
        void CompleteDownload();

        // the BitData of associate BitTask
        std::tr1::shared_ptr<BitData> bitdata_;
        // scattered block requests within some pieces
        BitRequestList scattered_request_;
        // PieceIndexSearcher ptr
        ScopePtr<PieceIndexSearcher> piece_index_searcher_;
        // pieces count of total task
        std::size_t pieces_count_;
        // block count of one piece
        std::size_t block_count_;
        // need download BitPieceMap, associate with need download files
        BitPieceMap need_download_;
        // downloading BitPieceMap
        BitPieceMap downloading_;
        // end downloading mode, every peer will request every left piece
        bool end_downloading_mode_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_DOWNLOAD_DISPATCHER_H

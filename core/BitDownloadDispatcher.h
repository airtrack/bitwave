#ifndef BIT_DOWNLOAD_DISPATCHER_H
#define BIT_DOWNLOAD_DISPATCHER_H

#include "BitPieceMap.h"
#include "BitRequestList.h"
#include "BitDownloadingInfo.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include <memory>

namespace bitwave {
namespace core {

    class BitData;
    class BitPeerData;

    class BitDownloadDispatcher :
        public BitDownloadingInfo::Observer, private NotCopyable
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

        BitDownloadDispatcher(
                const std::tr1::shared_ptr<BitData>& bitdata,
                BitDownloadingInfo *downloading_info);

        ~BitDownloadDispatcher();

        void DispatchRequestList(
                const std::tr1::shared_ptr<BitPeerData>& peer_data,
                BitRequestList& request_list);

        void ReturnRequest(BitRequestList& request_list,
                           BitRequestList::Iterator it);

        bool IsEndDownloadingMode() const
            { return end_downloading_mode_; }

    private:
        virtual void DownloadingNewPiece(std::size_t piece_index) { }
        virtual void CompleteNewPiece(std::size_t piece_index);
        virtual void DownloadingFailed(std::size_t piece_index) { }

        void DispatchScatteredRequest(
                const std::tr1::shared_ptr<BitPeerData>& peer_data,
                BitRequestList& request_list);
        void DispatchNewRequest(
                const std::tr1::shared_ptr<BitPeerData>& peer_data,
                BitRequestList& request_list);
        void ScatterRequestPiece(
                std::size_t piece_index,
                BitRequestList& list);
        void DeleteScatteredRequest(std::size_t piece_index);
        bool EnterEndDownloadMode();

        // bittask's bitdata
        std::tr1::shared_ptr<BitData> bitdata_;
        // task downloading information
        BitDownloadingInfo *downloading_info_;
        // scattered block requests
        BitRequestList scattered_request_;
        // PieceIndexSearcher ptr
        ScopePtr<PieceIndexSearcher> piece_index_searcher_;
        // pieces count of total task
        std::size_t pieces_count_;
        // block count of one piece
        std::size_t block_count_;
        // end downloading mode, every peer will request every left piece
        bool end_downloading_mode_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_DOWNLOAD_DISPATCHER_H

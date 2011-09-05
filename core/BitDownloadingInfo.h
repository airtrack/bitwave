#ifndef BIT_DOWNLOADING_INFO_H
#define BIT_DOWNLOADING_INFO_H

#include "BitPieceMap.h"
#include "../base/BaseTypes.h"
#include <memory>
#include <set>

namespace bitwave {
namespace core {

    class BitData;

    class BitDownloadingInfo : private NotCopyable
    {
    public:
        class Observer
        {
        public:
            virtual void DownloadingNewPiece(std::size_t piece_index) = 0;
            virtual void CompleteNewPiece(std::size_t piece_index) = 0;
            virtual void DownloadingFailed(std::size_t piece_index) = 0;
            virtual ~Observer() { }
        };

        explicit BitDownloadingInfo(
                const std::tr1::shared_ptr<BitData>& bitdata);

        void AddInfoObserver(Observer *observer)
            { observers_.insert(observer); }
        void RemoveInfoObserver(Observer *observer)
            { observers_.erase(observer); }

        const BitPieceMap& GetDownloaded() const
            { return downloaded_; }
        const BitPieceMap& GetNeedDownload() const
            { return need_download_; }
        const BitPieceMap& GetDownloading() const
            { return downloading_; }

        void MarkDownloading(std::size_t piece_index);
        void MarkDownloadComplete(std::size_t piece_index);
        void DownloadingFailed(std::size_t piece_index);

    private:
        void UpdateNeedDownload(
                const std::tr1::shared_ptr<BitData>& bitdata);

        BitPieceMap& downloaded_;
        BitPieceMap need_download_;
        BitPieceMap downloading_;

        std::set<Observer *> observers_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_DOWNLOADING_INFO_H

#include "BitUploadDispatcher.h"
#include "BitCache.h"

namespace bittorrent {
namespace core {

    BitUploadDispatcher::BitUploadDispatcher(
            const std::tr1::shared_ptr<BitCache>& cache)
        : upload_time_(TimeTraits::now()),
          cache_(cache)
    {
    }

    void BitUploadDispatcher::PendingUpload(std::size_t index,
                                            std::size_t begin,
                                            std::size_t length,
                                            const UploadCallback& callback)
    {
        pending_list_.push_back(PendingData(index, begin, length, callback));
    }

    void BitUploadDispatcher::ProcessUpload()
    {
        NormalTimeType now = TimeTraits::now();
        if (now - upload_time_ >= 1000)
        {
            std::size_t upload_count =
                pending_list_.size() >= 30 ? 2 : 1;
            ProcessPending(upload_count);
            upload_time_ = now;
        }
    }

    void BitUploadDispatcher::ProcessPending(std::size_t count)
    {
        while (!pending_list_.empty() && count > 0)
        {
            PendingData& data = pending_list_.front();
            cache_->Read(data.index, data.begin, data.length, data.callback);
            pending_list_.pop_front();
            --count;
        }
    }

} // namespace core
} // namespace bittorrent

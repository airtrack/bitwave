#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include "BitUploadDispatcher.h"
#include "BitCache.h"
#include "BitPeerConnection.h"
#include <functional>

using namespace std::tr1::placeholders;

namespace {
    const std::size_t quota_per_second = 2;
} // unnamed namespace

namespace bitwave {
namespace core {

    BitUploadDispatcher::BitUploadDispatcher(
            const std::tr1::shared_ptr<BitCache>& cache)
        : upload_time_(TimeTraits::now()),
          upload_blocks_quota_(quota_per_second),
          cache_(cache)
    {
    }

    void BitUploadDispatcher::PendingUpload(const ConnectionWeakPtr& weak_conn,
                                            int index, int begin, int length)
    {
        pending_list_.push_back(PendingData(weak_conn, index, begin, length));
    }

    void BitUploadDispatcher::ProcessUpload()
    {
        NormalTimeType now = TimeTraits::now();
        if (now - upload_time_ >= 1000)
        {
            std::size_t count = upload_blocks_quota_;
            upload_blocks_quota_ = quota_per_second;
            ProcessPending(count);
            upload_time_ = now;
        }
    }

    void BitUploadDispatcher::ProcessPending(std::size_t count)
    {
        while (!pending_list_.empty() && count > 0)
        {
            PendingData data = pending_list_.front();
            pending_list_.pop_front();

            if (!data.weak_conn.expired())
            {
                cache_->Read(data.index, data.begin, data.length,
                        std::tr1::bind(&BitUploadDispatcher::CacheReadCallback,
                            this, data, _1, _2));
                --count;
            }
        }
    }

    void BitUploadDispatcher::CacheReadCallback(const PendingData& data,
                                                bool read_ok,
                                                const char *block)
    {
        if (data.weak_conn.expired() ||
            !data.weak_conn.lock()->UploadBlock(
                data.index, data.begin, data.length, read_ok, block))
        {
            // upload failure, do not consume upload_blocks_quota_,
            // so increase upload_blocks_quota_, then next upload round
            // can consume the upload_blocks_quota_
            ++upload_blocks_quota_;
        }
    }

} // namespace core
} // namespace bitwave

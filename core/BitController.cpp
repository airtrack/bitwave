#include "BitController.h"
#include "BitData.h"
#include "BitTask.h"
#include "BitCache.h"
#include "BitUploadDispatcher.h"
#include "BitDownloadDispatcher.h"
#include <algorithm>
#include <functional>

namespace bitwave {
namespace core {

    void BitController::AddTask(const std::tr1::shared_ptr<BitTask>& task_ptr)
    {
        tasks_.push_back(task_ptr);

        std::tr1::shared_ptr<BitData> bitdata = task_ptr->GetBitData();
        Sha1Value info_hash = bitdata->GetInfoHash();

        DownloadDispatcherPtr down_dispatcher(new BitDownloadDispatcher(bitdata));
        download_dispatchers_.insert(std::make_pair(info_hash, down_dispatcher));

        TaskCachePtr cache(new BitCache(bitdata));
        task_caches_.insert(std::make_pair(info_hash, cache));

        UploadDispatcherPtr up_dispatcher(new BitUploadDispatcher(cache));
        upload_dispatchers_.insert(std::make_pair(info_hash, up_dispatcher));
    }

    std::tr1::shared_ptr<BitTask> BitController::GetTask(const Sha1Value& info_hash) const
    {
        Tasks::const_iterator it = std::find_if(tasks_.begin(), tasks_.end(),
                std::tr1::bind(&BitTask::IsSameInfoHash,
                    std::tr1::placeholders::_1, info_hash));

        if (it == tasks_.end())
            return std::tr1::shared_ptr<BitTask>();

        return *it;
    }

    std::size_t BitController::GetTaskCount() const
    {
        return tasks_.size();
    }

    bool BitController::AttachPeerToTask(const Sha1Value& info_hash,
                                         const std::tr1::shared_ptr<BitPeerConnection>& peer,
                                         std::tr1::shared_ptr<BitData> *bitdata)
    {
        assert(bitdata);
        std::tr1::shared_ptr<BitTask> task = GetTask(info_hash);
        if (!task)
            return false;

        *bitdata = task->GetBitData();
        task->AttachPeer(peer);
        return true;
    }

    std::tr1::shared_ptr<BitCache> BitController::GetTaskCache(
            const Sha1Value& info_hash) const
    {
        TaskCaches::const_iterator it = task_caches_.find(info_hash);
        if (it == task_caches_.end())
            return TaskCachePtr();
        return it->second;
    }

    std::tr1::shared_ptr<BitDownloadDispatcher> BitController::GetTaskDownloadDispather(
            const Sha1Value& info_hash) const
    {
        DownloadDispatchers::const_iterator it = download_dispatchers_.find(info_hash);
        if (it == download_dispatchers_.end())
            return DownloadDispatcherPtr();
        return it->second;
    }

    std::tr1::shared_ptr<BitUploadDispatcher> BitController::GetTaskUploadDispatcher(
            const Sha1Value& info_hash) const
    {
        UploadDispatchers::const_iterator it = upload_dispatchers_.find(info_hash);
        if (it == upload_dispatchers_.end())
            return UploadDispatcherPtr();
        return it->second;
    }

    void BitController::CompletePiece(const Sha1Value& info_hash,
                                      std::size_t piece_index)
    {
        std::tr1::shared_ptr<BitTask> task = GetTask(info_hash);
        assert(task);
        task->CompletePiece(piece_index);
    }

    void BitController::CompleteDownload(const Sha1Value& info_hash)
    {
        std::tr1::shared_ptr<BitTask> task = GetTask(info_hash);
        assert(task);
        task->CompleteDownload();

        TaskCachePtr cache = GetTaskCache(info_hash);
        assert(cache);
        cache->FlushToFile();
    }

    void BitController::LetAllPeerRequestPiece(const Sha1Value& info_hash)
    {
        std::tr1::shared_ptr<BitTask> task = GetTask(info_hash);
        assert(task);
        task->AllPeerRequestPiece();
    }

    void BitController::Process()
    {
        for (TaskCaches::iterator it = task_caches_.begin();
                it != task_caches_.end(); ++it)
        {
            it->second->ProcessCache();
        }

        for (UploadDispatchers::iterator it = upload_dispatchers_.begin();
                it != upload_dispatchers_.end(); ++it)
        {
            it->second->ProcessUpload();
        }
    }

} // namespace core
} // namespace bitwave

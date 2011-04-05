#include "BitController.h"
#include "BitData.h"
#include "BitTask.h"
#include "BitDownloadDispatcher.h"
#include <algorithm>
#include <functional>

namespace bittorrent {
namespace core {

    void BitController::AddTask(const std::tr1::shared_ptr<BitTask>& task_ptr)
    {
        tasks_.push_back(task_ptr);

        std::tr1::shared_ptr<BitData> bitdata = task_ptr->GetBitData();
        DownloadDispatcherPtr dispatcher(new BitDownloadDispatcher(bitdata));
        Sha1Value info_hash = bitdata->GetInfoHash();
        dispatchers_.insert(std::make_pair(info_hash, dispatcher));
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

    std::tr1::shared_ptr<BitDownloadDispatcher> BitController::GetTaskDownloadDispather(
            const Sha1Value& info_hash)
    {
        DownloadDispatchers::iterator it = dispatchers_.find(info_hash);
        if (it == dispatchers_.end())
            return DownloadDispatcherPtr();
        return it->second;
    }

} // namespace core
} // namespace bittorrent

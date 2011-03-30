#include "BitController.h"
#include "BitTask.h"
#include <algorithm>
#include <functional>

namespace bittorrent {
namespace core {

    void BitController::AddTask(const TaskPtr& task_ptr)
    {
        tasks_.push_back(task_ptr);
    }

    std::size_t BitController::GetTaskCount() const
    {
        return tasks_.size();
    }

    BitController::TaskPtr BitController::GetTask(const Sha1Value& info_hash) const
    {
        Tasks::const_iterator it = std::find_if(tasks_.begin(), tasks_.end(),
                std::tr1::bind(&BitTask::IsSameInfoHash,
                    std::tr1::placeholders::_1, info_hash));

        if (it == tasks_.end())
            return TaskPtr();

        return *it;
    }

} // namespace core
} // namespace bittorrent

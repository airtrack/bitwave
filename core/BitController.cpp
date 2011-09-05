#include "BitController.h"
#include "BitTask.h"
#include <algorithm>

namespace bitwave {
namespace core {

    void BitController::AddTask(const std::tr1::shared_ptr<BitTask>& task_ptr)
    {
        tasks_.push_back(task_ptr);
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

    void BitController::Process()
    {
        std::for_each(tasks_.begin(), tasks_.end(),
                std::tr1::bind(&BitTask::ProcessTask, std::tr1::placeholders::_1));
    }

} // namespace core
} // namespace bitwave

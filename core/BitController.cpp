#include "BitController.h"
#include "BitTask.h"

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

} // namespace core
} // namespace bittorrent

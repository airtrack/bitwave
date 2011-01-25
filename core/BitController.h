#ifndef BIT_CONTROLLER_H
#define BIT_CONTROLLER_H

#include "../base/BaseTypes.h"

#include <memory>
#include <vector>

namespace bittorrent {
namespace core {

    class BitTask;

    // this class manage and control all created BitTasks
    class BitController : private NotCopyable
    {
    public:
        typedef std::tr1::shared_ptr<BitTask> TaskPtr;
        typedef std::vector<TaskPtr> Tasks;

        void AddTask(const TaskPtr& task_ptr);
        std::size_t GetTaskCount() const;

    private:
        Tasks tasks_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_CONTROLLER_H

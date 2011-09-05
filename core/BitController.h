#ifndef BIT_CONTROLLER_H
#define BIT_CONTROLLER_H

#include "../base/BaseTypes.h"
#include "../sha1/Sha1Value.h"
#include <memory>
#include <vector>
#include <map>

namespace bitwave {
namespace core {

    class BitTask;

    // this class manage and control all created BitTasks
    class BitController : private NotCopyable
    {
    public:
        void AddTask(const std::tr1::shared_ptr<BitTask>& task_ptr);
        std::tr1::shared_ptr<BitTask> GetTask(const Sha1Value& info_hash) const;
        std::size_t GetTaskCount() const;
        void Process();

    private:
        typedef std::vector<std::tr1::shared_ptr<BitTask>> Tasks;
        Tasks tasks_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_CONTROLLER_H

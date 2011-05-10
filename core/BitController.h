#ifndef BIT_CONTROLLER_H
#define BIT_CONTROLLER_H

#include "../base/BaseTypes.h"
#include "../sha1/Sha1Value.h"
#include <memory>
#include <vector>
#include <map>

namespace bittorrent {
namespace core {

    class BitTask;
    class BitData;
    class BitCache;
    class BitPeerConnection;
    class BitDownloadDispatcher;

    // this class manage and control all created BitTasks
    class BitController : private NotCopyable
    {
    public:
        void AddTask(const std::tr1::shared_ptr<BitTask>& task_ptr);
        std::tr1::shared_ptr<BitTask> GetTask(const Sha1Value& info_hash) const;
        std::size_t GetTaskCount() const;

        bool AttachPeerToTask(const Sha1Value& info_hash,
                              const std::tr1::shared_ptr<BitPeerConnection>& peer,
                              std::tr1::shared_ptr<BitData> *bitdata);

        std::tr1::shared_ptr<BitDownloadDispatcher> GetTaskDownloadDispather(
                const Sha1Value& info_hash);

    private:
        typedef std::vector<std::tr1::shared_ptr<BitTask>> Tasks;
        typedef std::tr1::shared_ptr<BitCache> TaskCachePtr;
        typedef std::map<Sha1Value, TaskCachePtr> TaskCaches;
        typedef std::tr1::shared_ptr<BitDownloadDispatcher> DownloadDispatcherPtr;
        typedef std::map<Sha1Value, DownloadDispatcherPtr> DownloadDispatchers;

        Tasks tasks_;
        TaskCaches task_caches_;
        DownloadDispatchers dispatchers_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_CONTROLLER_H

#ifndef BIT_UPLOAD_DISPATCHER_H
#define BIT_UPLOAD_DISPATCHER_H

#include "../base/BaseTypes.h"
#include "../timer/TimeTraits.h"
#include <list>
#include <memory>
#include <functional>

namespace bitwave {
namespace core {

    class BitCache;

    class BitUploadDispatcher : private NotCopyable
    {
    public:
        typedef std::tr1::function<void (bool, const char *)> UploadCallback;

        explicit BitUploadDispatcher(
                const std::tr1::shared_ptr<BitCache>& cache);

        void PendingUpload(std::size_t index,
                           std::size_t begin,
                           std::size_t length,
                           const UploadCallback& callback);

        void ProcessUpload();

    private:
        typedef time_traits<NormalTimeType> TimeTraits;

        struct PendingData
        {
            PendingData(std::size_t i, std::size_t b, std::size_t l,
                        const UploadCallback& c)
                : index(i),
                  begin(b),
                  length(l),
                  callback(c)
            {
            }

            std::size_t index;
            std::size_t begin;
            std::size_t length;
            UploadCallback callback;
        };

        void ProcessPending(std::size_t count);

        NormalTimeType upload_time_;
        std::list<PendingData> pending_list_;
        std::tr1::shared_ptr<BitCache> cache_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_UPLOAD_DISPATCHER_H

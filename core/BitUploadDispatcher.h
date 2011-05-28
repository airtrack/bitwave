#ifndef BIT_UPLOAD_DISPATCHER_H
#define BIT_UPLOAD_DISPATCHER_H

#include "../base/BaseTypes.h"
#include "../timer/TimeTraits.h"
#include <list>
#include <memory>

namespace bitwave {
namespace core {

    class BitCache;
    class BitPeerConnection;

    class BitUploadDispatcher : private NotCopyable
    {
    public:
        typedef std::tr1::weak_ptr<BitPeerConnection> ConnectionWeakPtr;

        explicit BitUploadDispatcher(
                const std::tr1::shared_ptr<BitCache>& cache);

        void PendingUpload(const ConnectionWeakPtr& weak_conn,
                           int index, int begin, int length);

        void ProcessUpload();

    private:
        typedef time_traits<NormalTimeType> TimeTraits;

        struct PendingData
        {
            PendingData(const ConnectionWeakPtr& wc,
                        int i, int b, int l)
                : weak_conn(wc),
                  index(i),
                  begin(b),
                  length(l)
            {
            }

            ConnectionWeakPtr weak_conn;
            int index;
            int begin;
            int length;
        };

        void ProcessPending(std::size_t count);
        void CacheReadCallback(const PendingData& data,
                               bool read_ok,
                               const char *block);

        NormalTimeType upload_time_;
        std::list<PendingData> pending_list_;
        std::tr1::shared_ptr<BitCache> cache_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_UPLOAD_DISPATCHER_H

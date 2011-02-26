#ifndef BIT_TASK_H
#define BIT_TASK_H

#include "BitRepository.h"
#include "../base/BaseTypes.h"
#include "../net/IoService.h"
#include <memory>
#include <vector>

namespace bittorrent {
namespace core {

    class BitTrackerConnection;

    // task class to control a bittorrent download task
    class BitTask : private NotCopyable
    {
    public:
        // create a BitTask associate with the bitdata
        BitTask(const BitRepository::BitDataPtr& bitdata,
                net::IoService& io_service);

    private:
        typedef std::tr1::shared_ptr<BitTrackerConnection> TrackerConnPtr;
        typedef std::vector<TrackerConnPtr> Trackers;

        void CreateTrackerConnection();
        void UpdateTrackerInfo();

        net::IoService& io_service_;
        BitRepository::BitDataPtr bitdata_;
        Trackers trackers_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_TASK_H

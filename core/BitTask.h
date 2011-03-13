#ifndef BIT_TASK_H
#define BIT_TASK_H

#include "BitRepository.h"
#include "BitPeerConnection.h"
#include "../base/BaseTypes.h"
#include "../net/IoService.h"
#include <set>
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

        // attach peer to this task
        void AttachPeer(const std::tr1::shared_ptr<BitPeerConnection>& peer);

    private:
        typedef std::tr1::shared_ptr<BitTrackerConnection> TrackerConnPtr;
        typedef std::vector<TrackerConnPtr> TaskTrackers;

        class TaskPeers : public PeerConnectionOwner, private NotCopyable
        {
        public:
            void AddPeer(const std::tr1::shared_ptr<BitPeerConnection>& peer)
            {
                peers_.insert(peer);
            }

        private:
            virtual void LetMeLeave(const std::tr1::shared_ptr<BitPeerConnection>& child)
            {
                peers_.erase(child);
            }

            std::set<std::tr1::shared_ptr<BitPeerConnection>> peers_;
        };

        void CreateTrackerConnection();
        void UpdateTrackerInfo();

        net::IoService& io_service_;
        BitRepository::BitDataPtr bitdata_;
        TaskTrackers trackers_;
        TaskPeers peers_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_TASK_H

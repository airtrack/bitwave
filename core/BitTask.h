#ifndef BIT_TASK_H
#define BIT_TASK_H

#include "BitPeerConnection.h"
#include "BitPeerCreateStrategy.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../net/IoService.h"
#include "../timer/Timer.h"
#include <set>
#include <memory>
#include <vector>

namespace bittorrent {
namespace core {

    class BitData;
    class BitTrackerConnection;

    // task class to control a bittorrent download task
    class BitTask : private NotCopyable
    {
    public:
        // create a BitTask associate with the bitdata
        BitTask(const std::tr1::shared_ptr<BitData>& bitdata,
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

            std::size_t GetPeerCount() const
            {
                return peers_.size();
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
        void InitCreatePeersTimer();
        void PrepareTimerDeadline();
        void OnTimer();
        void CreateTaskPeer(std::size_t count);

        net::IoService& io_service_;
        Timer create_peers_timer_;
        ScopePtr<BitPeerCreateStrategy> create_strategy_;
        std::tr1::shared_ptr<BitData> bitdata_;
        TaskTrackers trackers_;
        TaskPeers peers_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_TASK_H

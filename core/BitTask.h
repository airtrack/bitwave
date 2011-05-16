#ifndef BIT_TASK_H
#define BIT_TASK_H

#include "BitPeerConnection.h"
#include "BitPeerCreateStrategy.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../net/IoService.h"
#include "../timer/Timer.h"
#include "../sha1/Sha1Value.h"
#include <set>
#include <memory>
#include <vector>
#include <algorithm>

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

        ~BitTask();

        // attach peer to this task
        void AttachPeer(const std::tr1::shared_ptr<BitPeerConnection>& peer);

        // let all peer to send request
        void AllPeerRequestPiece();

        // complete download the piece_index piece
        void CompletePiece(std::size_t piece_index);

        // task download completely
        void CompleteDownload();

        // the task info_hash is equal to param info_hash
        bool IsSameInfoHash(const Sha1Value& info_hash) const;

        // get associate BitData of the task
        std::tr1::shared_ptr<BitData> GetBitData() const;

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

            template<typename Operator>
            void ForEach(const Operator& op)
            {
                std::for_each(peers_.begin(), peers_.end(), op);
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
        void ClearTimer();
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

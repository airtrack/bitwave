#ifndef BIT_TASK_H
#define BIT_TASK_H

#include "BitPeerConnection.h"
#include "BitPeerCreateStrategy.h"
#include "BitDownloadingInfo.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../net/IoService.h"
#include "../timer/Timer.h"
#include "../sha1/Sha1Value.h"
#include <assert.h>
#include <set>
#include <memory>
#include <vector>
#include <algorithm>

namespace bitwave {
namespace core {

    class BitData;
    class BitCache;
    class BitPeerConnection;
    class BitTrackerConnection;
    class BitUploadDispatcher;
    class BitDownloadDispatcher;

    // task class to control a bitwave download task
    class BitTask : private NotCopyable
    {
    public:
        // create a BitTask associate with the bitdata
        BitTask(const std::tr1::shared_ptr<BitData>& bitdata,
                net::IoService& io_service);

        ~BitTask();

        // attach peer to this task
        void AttachPeer(const std::tr1::shared_ptr<BitPeerConnection>& peer);

        // the task info_hash is equal to param info_hash
        bool IsSameInfoHash(const Sha1Value& info_hash) const;

        void ProcessTask();

    private:
        friend class TaskPeers;
        typedef std::tr1::shared_ptr<BitTrackerConnection> TrackerConnPtr;
        typedef std::vector<TrackerConnPtr> TaskTrackers;

        class TaskPeers : public PeerConnectionOwner, private NotCopyable
        {
        public:
            TaskPeers()
                : task_(0)
            {
            }

            void SetTask(BitTask *task)
            {
                assert(task);
                task_ = task;
            }

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
            virtual bool NotifyInfoHash(const std::tr1::shared_ptr<BitPeerConnection>& child, const Sha1Value& info_hash)
            {
                return true;
            }

            virtual void NotifyHandshakeOk(const std::tr1::shared_ptr<BitPeerConnection>& child)
            {
                assert(task_);
                task_->AddDownloadingInfoObserver(child.get());
                task_->SetPeerConnectionBaseData(child.get());
            }

            virtual void NotifyConnectionDrop(const std::tr1::shared_ptr<BitPeerConnection>& child)
            {
                assert(task_);
                task_->RemoveDownloadingInfoObserver(child.get());
                peers_.erase(child);
            }

            BitTask *task_;
            std::set<std::tr1::shared_ptr<BitPeerConnection>> peers_;
        };

        class DownloadedUpdater : public BitDownloadingInfo::Observer
        {
        public:
            explicit DownloadedUpdater(const std::tr1::shared_ptr<BitData>& bitdata);

            virtual void DownloadingNewPiece(std::size_t piece_index) { }
            virtual void CompleteNewPiece(std::size_t piece_index);
            virtual void DownloadingFailed(std::size_t piece_index) { }

        private:
            std::tr1::shared_ptr<BitData> bitdata_;
            std::size_t piece_length_;
        };

        void CreateTrackerConnection();
        void UpdateTrackerInfo();
        void InitCreatePeersTimer();
        void PrepareTimerDeadline();
        void ClearTimer();
        void OnTimer();
        void CreateTaskPeer(std::size_t count);
        void AddDownloadingInfoObserver(BitPeerConnection *observer);
        void RemoveDownloadingInfoObserver(BitPeerConnection *observer);
        void SetPeerConnectionBaseData(BitPeerConnection *peer_conn);

        net::IoService& io_service_;
        Timer create_peers_timer_;
        ScopePtr<BitPeerCreateStrategy> create_strategy_;
        std::tr1::shared_ptr<BitData> bitdata_;

        TaskTrackers trackers_;
        TaskPeers peers_;
        BitDownloadingInfo downloading_info_;
        DownloadedUpdater downloaded_updater_;

        std::tr1::shared_ptr<BitCache> cache_;
        std::tr1::shared_ptr<BitUploadDispatcher> uploader_;
        std::tr1::shared_ptr<BitDownloadDispatcher> downloader_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_TASK_H

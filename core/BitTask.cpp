#include "BitTask.h"
#include "BitData.h"
#include "BitCache.h"
#include "BitTrackerConnection.h"
#include "BitUploadDispatcher.h"
#include "BitDownloadDispatcher.h"
#include "bencode/MetainfoFile.h"
#include "../net/TimerService.h"
#include <assert.h>
#include <functional>

namespace bitwave {
namespace core {

    BitTask::DownloadedUpdater::DownloadedUpdater(
            const std::tr1::shared_ptr<BitData>& bitdata)
        : bitdata_(bitdata),
          piece_length_(bitdata->GetPieceLength())
    {
    }

    void BitTask::DownloadedUpdater::CompleteNewPiece(
            std::size_t piece_index)
    {
        bitdata_->IncreaseDownloaded(piece_length_);
    }

    BitTask::BitTask(const std::tr1::shared_ptr<BitData>& bitdata,
                     net::IoService& io_service)
        : io_service_(io_service),
          bitdata_(bitdata),
          downloading_info_(bitdata),
          downloaded_updater_(bitdata),
          cache_(new BitCache(bitdata, &downloading_info_)),
          uploader_(new BitUploadDispatcher(cache_)),
          downloader_(new BitDownloadDispatcher(bitdata, &downloading_info_))
    {
        peers_.SetTask(this);
        downloading_info_.AddInfoObserver(&downloaded_updater_);

        BitPeerCreateStrategy *strategy = CreateDefaultPeerCreateStartegy();
        create_strategy_.Reset(strategy);

        InitCreatePeersTimer();
        CreateTrackerConnection();
    }

    BitTask::~BitTask()
    {
        downloading_info_.RemoveInfoObserver(&downloaded_updater_);
        ClearTimer();
    }

    void BitTask::AttachPeer(const std::tr1::shared_ptr<BitPeerConnection>& peer)
    {
        peers_.AddPeer(peer);
        peer->SetOwner(&peers_);
        peer->SetBitData(bitdata_);
    }

    bool BitTask::IsSameInfoHash(const Sha1Value& info_hash) const
    {
        return bitdata_->GetInfoHash() == info_hash;
    }

    void BitTask::ProcessTask()
    {
        cache_->ProcessCache();
        uploader_->ProcessUpload();
    }

    void BitTask::CreateTrackerConnection()
    {
        typedef std::vector<std::string> AnnounceList;
        const bentypes::MetainfoFile *info = bitdata_->GetMetainfoFile();

        AnnounceList announce;
        info->GetAnnounce(&announce);

        for (AnnounceList::iterator it = announce.begin();
                it != announce.end(); ++it)
        {
            try
            {
                BitTrackerConnection *btc = new BitTrackerConnection(
                        *it, bitdata_, io_service_);
                TrackerConnPtr ptr(btc);
                trackers_.push_back(ptr);
            }
            catch (...)
            {
                // we continue create tracker connection
            }
        }
    }

    void BitTask::UpdateTrackerInfo()
    {
        std::for_each(
                trackers_.begin(),
                trackers_.end(),
                std::tr1::bind(&BitTrackerConnection::UpdateTrackerInfo,
                    std::tr1::placeholders::_1));
    }

    void BitTask::InitCreatePeersTimer()
    {
        create_peers_timer_.SetCallback(
                std::tr1::bind(&BitTask::OnTimer, this));

        net::ServicePtr<net::TimerService> timer_service(io_service_);
        assert(timer_service);
        timer_service->AddTimer(&create_peers_timer_);
        PrepareTimerDeadline();
    }

    void BitTask::PrepareTimerDeadline()
    {
        std::size_t peer_count = peers_.GetPeerCount();
        std::size_t create_interval = create_strategy_->CreatePeerInterval(peer_count);
        create_peers_timer_.SetDeadline(create_interval);
    }

    void BitTask::ClearTimer()
    {
        net::ServicePtr<net::TimerService> timer_service(io_service_);
        assert(timer_service);
        timer_service->DelTimer(&create_peers_timer_);
    }

    void BitTask::OnTimer()
    {
        std::size_t peer_count = peers_.GetPeerCount();
        std::size_t unused_count = bitdata_->GetUnusedListenInfo().size();
        std::size_t create_count = create_strategy_->CreatePeerCount(peer_count, unused_count);
        CreateTaskPeer(create_count);
        PrepareTimerDeadline();
    }

    void BitTask::CreateTaskPeer(std::size_t count)
    {
        BitData::ListenInfoSet& unused = bitdata_->GetUnusedListenInfo();
        std::size_t total_unused = unused.size();
        count = count > total_unused ? total_unused : count;

        std::size_t i = 0;
        BitData::ListenInfoSet::iterator it = unused.begin();
        while (i < count)
        {
            net::Address address(it->ip);
            net::Port port(it->port);
            BitPeerConnection *ptr = new BitPeerConnection(bitdata_, io_service_, &peers_);
            std::tr1::shared_ptr<BitPeerConnection> peer(ptr);
            peers_.AddPeer(peer);
            peer->Connect(address, port);
            ++i;
            ++it;
        }

        // move unused to used
        BitData::ListenInfoSet& used = bitdata_->GetUsedListenInfo();
        used.insert(unused.begin(), it);
        unused.erase(unused.begin(), it);
    }

    void BitTask::AddDownloadingInfoObserver(BitPeerConnection *observer)
    {
        assert(observer);
        downloading_info_.AddInfoObserver(observer);
    }

    void BitTask::RemoveDownloadingInfoObserver(BitPeerConnection *observer)
    {
        assert(observer);
        downloading_info_.RemoveInfoObserver(observer);
    }

    void BitTask::SetPeerConnectionBaseData(BitPeerConnection *peer_conn)
    {
        peer_conn->SetCache(cache_);
        peer_conn->SetUploadDispatcher(uploader_);
        peer_conn->SetDownloadDispatcher(downloader_);
    }

} // namespace core
} // namespace bitwave

#include "BitTask.h"
#include "BitData.h"
#include "BitTrackerConnection.h"
#include "bencode/MetainfoFile.h"
#include <algorithm>

namespace bittorrent {
namespace core {

    BitTask::BitTask(const std::tr1::shared_ptr<BitData>& bitdata,
                     net::IoService& io_service)
        : io_service_(io_service),
          bitdata_(bitdata)
    {
        CreateTrackerConnection();
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

} // namespace core
} // namespace bittorrent

#include "BitData.h"
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <functional>

namespace bittorrent {
namespace core {

    BitData::BitData(const std::string& torrent_file)
        : torrent_file_(torrent_file),
          uploaded_(0),
          downloaded_(0)
    {
        metainfo_file_.Reset(new bentypes::MetainfoFile(torrent_file_.c_str()));
        downloaded_map_.Reset(new BitPieceMap(GetPieceCount()));

        std::pair<const char *, const char *> info_data = metainfo_file_->GetRawInfoValue();
        info_hash_ = Sha1Value(info_data.first, info_data.second);

        peer_id_ = "-AT0001-000000000000";
    }

    const bentypes::MetainfoFile * BitData::GetMetainfoFile() const
    {
        return metainfo_file_.Get();
    }

    std::string BitData::GetTorrentFile() const
    {
        return torrent_file_;
    }

    Sha1Value BitData::GetInfoHash() const
    {
        return info_hash_;
    }

    std::string BitData::GetPeerId() const
    {
        return peer_id_;
    }

    std::size_t BitData::GetPieceCount() const
    {
        return metainfo_file_->PiecesCount();
    }

    long long BitData::GetUploaded() const
    {
        return uploaded_;
    }

    long long BitData::GetDownloaded() const
    {
        return downloaded_;
    }

    long long BitData::GetTotalSize() const
    {
        long long size = metainfo_file_->Length();
        if (size > 0)
            return size;

        // calculate all files size
        typedef std::vector<bentypes::MetainfoFile::FileInfo> FilesInfo;
        FilesInfo files_info;
        metainfo_file_->Files(&files_info);
        for (FilesInfo::iterator it = files_info.begin();
                it != files_info.end(); ++it)
        {
            size += it->length;
        }

        return size;
    }

    bool BitData::IsDownloadComplete() const
    {
        return false;
    }

    const BitPieceMap& BitData::GetPieceMap() const
    {
        return *downloaded_map_;
    }

    void BitData::AddPeerListenInfo(unsigned long ip, unsigned short port)
    {
        PeerListenInfo info(ip, port);
        if (used_peers_.find(info) != used_peers_.end())
            return ;
        unused_peers_.insert(info);
    }

    BitData::ListenInfoSet& BitData::GetUnusedListenInfo()
    {
        return unused_peers_;
    }

    BitData::ListenInfoSet& BitData::GetUsedListenInfo()
    {
        return used_peers_;
    }

    void BitData::MergeToUnusedListenInfo(const ListenInfoSet& info_set)
    {
        std::for_each(info_set.begin(), info_set.end(),
                std::tr1::bind(&ListenInfoSet::insert, &unused_peers_,
                    std::tr1::placeholders::_1));
    }

    void BitData::MergeToUsedListenInfo(const ListenInfoSet& info_set)
    {
        std::for_each(info_set.begin(), info_set.end(),
                std::tr1::bind(&ListenInfoSet::insert, &used_peers_,
                    std::tr1::placeholders::_1));
    }

    void BitData::ClearUnusedListenInfo()
    {
        unused_peers_.clear();
    }

    void BitData::ClearUsedListenInfo()
    {
        used_peers_.clear();
    }

    void BitData::AddPeerData(const std::tr1::shared_ptr<BitPeerData>& peer_data)
    {
        peer_data_set_.insert(peer_data);
    }

    void BitData::DelPeerData(const std::tr1::shared_ptr<BitPeerData>& peer_data)
    {
        peer_data_set_.erase(peer_data);
    }

    BitData::PeerDataSet& BitData::GetPeerDataSet()
    {
        return peer_data_set_;
    }

} // namespace core
} // namespace bittorrent

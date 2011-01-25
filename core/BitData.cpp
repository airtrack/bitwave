#include "BitData.h"
#include <assert.h>
#include <string.h>

namespace bittorrent {
namespace core {

    BitData::BitData(const std::string& torrent_file)
        : torrent_file_(torrent_file),
          uploaded_(0),
          downloaded_(0)
    {
        metainfo_file_.Reset(new bentypes::MetainfoFile(torrent_file_.c_str()));

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

    BitData::PeerDataPtr BitData::AddPeerData(const PeerKey& key)
    {
        PeerDataPtr ptr = GetPeerData(key);
        if (ptr)
            return ptr;

        ptr.reset(new BitPeerData(key.ip, key.port));
        peers_data_.insert(std::make_pair(key, ptr));
        return ptr;
    }

    BitData::PeerDataPtr BitData::GetPeerData(const PeerKey& key) const
    {
        PeersData::const_iterator it = peers_data_.find(key);
        if (it == peers_data_.end())
            return PeerDataPtr();
        return it->second;
    }

    void BitData::GetAllPeerData(std::vector<PeerDataPtr>& peers) const
    {
        peers.reserve(peers_data_.size());
        for (PeersData::const_iterator it = peers_data_.begin();
                it != peers_data_.end(); ++it)
        {
            peers.push_back(it->second);
        }
    }

} // namespace core
} // namespace bittorrent

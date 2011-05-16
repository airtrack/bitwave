#include "BitData.h"
#include <assert.h>
#include <string.h>
#include <algorithm>
#include <functional>
#include <iterator>

namespace bittorrent {
namespace core {

    BitData::BitData(const std::string& torrent_file)
        : torrent_file_(torrent_file),
          uploaded_(0),
          downloaded_(0),
          total_size_(0),
          current_download_(0)
    {
        metainfo_file_.Reset(new bentypes::MetainfoFile(torrent_file_.c_str()));
        downloaded_map_.Reset(new BitPieceMap(GetPieceCount()));

        std::pair<const char *, const char *> info_data = metainfo_file_->GetRawInfoValue();
        info_hash_ = Sha1Value(info_data.first, info_data.second);

        peer_id_ = "-AT0001-000000000000";

        PrepareDownloadFiles();
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

    std::size_t BitData::GetPieceLength() const
    {
        return metainfo_file_->PieceLength();
    }

    long long BitData::GetUploaded() const
    {
        return uploaded_;
    }

    void BitData::IncreaseUploaded(long long inc)
    {
        uploaded_ += inc;
    }

    long long BitData::GetDownloaded() const
    {
        return downloaded_;
    }

    void BitData::IncreaseDownloaded(long long inc)
    {
        downloaded_ += inc;
    }

    long long BitData::GetTotalSize() const
    {
        return total_size_;
    }

    long long BitData::GetCurrentDownload() const
    {
        return current_download_;
    }

    void BitData::IncreaseCurrentDownload(long long inc)
    {
        current_download_ += inc;
    }

    bool BitData::IsDownloadComplete() const
    {
        return downloaded_ >= total_size_;
    }

    BitPieceMap& BitData::GetPieceMap() const
    {
        return *downloaded_map_;
    }

    const BitData::DownloadFiles& BitData::GetFilesInfo() const
    {
        return download_files_;
    }

    void BitData::SetBasePath(const std::string& path)
    {
        base_path_ = path;
    }

    std::string BitData::GetBasePath() const
    {
        return base_path_;
    }

    void BitData::SelectFile(std::size_t file_index, bool download)
    {
        if (file_index < download_files_.size())
        {
            DownloadFiles::iterator it = download_files_.begin();
            std::advance(it, file_index);
            DoSelectFile(it, download);
        }
    }

    void BitData::SelectAllFile(bool download)
    {
        DownloadFiles::iterator it = download_files_.begin();
        DownloadFiles::iterator end = download_files_.end();
        for (; it != end; ++it)
            DoSelectFile(it, download);
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

    void BitData::PrepareDownloadFiles()
    {
        typedef std::vector<bentypes::MetainfoFile::FileInfo> FilesInfo;

        FilesInfo files_info;
        metainfo_file_->Files(&files_info);

        for (FilesInfo::iterator it = files_info.begin();
                it != files_info.end(); ++it)
        {
            DownloadFileInfo file(false, it->length);
            std::vector<std::string>::iterator i = it->path.begin();
            std::vector<std::string>::iterator end = it->path.end();
            for (; i != end; ++i)
            {
                file.file_path.append("\\");
                file.file_path.append(*i);
            }

            download_files_.push_back(file);
        }
    }

    void BitData::DoSelectFile(DownloadFiles::iterator it, bool download)
    {
        if (it->is_download != download)
        {
            it->is_download = download;
            if (download)
                total_size_ += it->length;
            else
                total_size_ -= it->length;
        }
    }

} // namespace core
} // namespace bittorrent

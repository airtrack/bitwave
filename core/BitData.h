#ifndef BIT_DATA_H
#define BIT_DATA_H

#include "BitPieceMap.h"
#include "BitPeerData.h"
#include "bencode/MetainfoFile.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../sha1/Sha1Value.h"
#include <set>
#include <memory>
#include <string>
#include <vector>

namespace bitwave {
namespace core {

    // a class store bitwave task data info
    class BitData : private NotCopyable
    {
    public:
        struct PeerListenInfo
        {
            PeerListenInfo(unsigned long i, unsigned short p)
                : ip(i), port(p)
            {
            }

            friend bool operator < (const PeerListenInfo& left,
                                    const PeerListenInfo& right)
            {
                if (left.ip == right.ip)
                    return left.port < right.port;
                return left.ip < right.ip;
            }

            unsigned long ip;
            unsigned short port;
        };

        struct DownloadFileInfo
        {
            DownloadFileInfo(bool download, long long len)
                : is_download(download),
                  length(len)
            {
            }

            bool is_download;       // file is download or not
            long long length;       // file total length in bytes
            std::string file_path;  // file relative path
        };

        typedef std::set<PeerListenInfo> ListenInfoSet;
        typedef std::set<std::tr1::shared_ptr<BitPeerData>> PeerDataSet;
        typedef std::vector<DownloadFileInfo> DownloadFiles;

        // construct a new BitTask's BitData
        explicit BitData(const std::string& torrent_file);

        // get MetainfoFile ptr
        const bentypes::MetainfoFile * GetMetainfoFile() const;

        // get the bitwave task torrent file path
        std::string GetTorrentFile() const;

        // get the BitData info hash of torrent file
        Sha1Value GetInfoHash() const;

        // get the bitwave task peer_id, the return value is string, store
        // the peer_id, 20 length
        std::string GetPeerId() const;

        // get piece count of the data
        std::size_t GetPieceCount() const;

        // get length bytes of one piece
        std::size_t GetPieceLength() const;

        // get total uploaded bytes
        long long GetUploaded() const;

        void IncreaseUploaded(long long inc);

        // get total downloaded bytes
        long long GetDownloaded() const;

        void IncreaseDownloaded(long long inc);

        // get total size bytes of need download
        long long GetTotalSize() const;

        long long GetCurrentDownload() const;

        void IncreaseCurrentDownload(long long inc);

        // download is complete or not
        bool IsDownloadComplete() const;

        // get downloaded piece map
        BitPieceMap& GetPieceMap() const;

        // get files info
        const DownloadFiles& GetFilesInfo() const;

        // set all downloaded files base path
        void SetBasePath(const std::string& path);

        // get all downloaded files base path
        std::string GetBasePath() const;

        // select file download or not
        void SelectFile(std::size_t file_index, bool download);

        // select all files download or not
        void SelectAllFile(bool download);

        // manage PeerListenInfo
        void AddPeerListenInfo(unsigned long ip, unsigned short port);
        ListenInfoSet& GetUnusedListenInfo();
        ListenInfoSet& GetUsedListenInfo();
        void MergeToUnusedListenInfo(const ListenInfoSet& info_set);
        void MergeToUsedListenInfo(const ListenInfoSet& info_set);
        void ClearUnusedListenInfo();
        void ClearUsedListenInfo();

        // manage PeerDataSet
        void AddPeerData(const std::tr1::shared_ptr<BitPeerData>& peer_data);
        void DelPeerData(const std::tr1::shared_ptr<BitPeerData>& peer_data);
        PeerDataSet& GetPeerDataSet();

    private:
        void PrepareDownloadFiles();
        void DoSelectFile(DownloadFiles::iterator it, bool download);

        typedef ScopePtr<bentypes::MetainfoFile> MetaInfoPtr;
        typedef ScopePtr<BitPieceMap> PieceMapPtr;

        // base data
        Sha1Value info_hash_;
        std::string torrent_file_;
        std::string peer_id_;
        std::size_t piece_length_;
        std::size_t piece_count_;
        long long uploaded_;
        long long downloaded_;
        long long total_size_;
        long long current_download_;

        MetaInfoPtr metainfo_file_;
        PieceMapPtr downloaded_map_;
        ListenInfoSet unused_peers_;
        ListenInfoSet used_peers_;
        PeerDataSet peer_data_set_;
        DownloadFiles download_files_;
        std::string base_path_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_DATA_H

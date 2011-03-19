#ifndef BIT_DATA_H
#define BIT_DATA_H

#include "BitPeerData.h"
#include "bencode/MetainfoFile.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../sha1/Sha1Value.h"
#include <set>
#include <memory>
#include <string>
#include <vector>

namespace bittorrent {
namespace core {

    // a class store bittorrent task data info
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

        typedef std::set<PeerListenInfo> ListenInfoSet;
        typedef std::set<std::tr1::shared_ptr<BitPeerData>> PeerDataSet;

        // construct a new BitTask's BitData
        explicit BitData(const std::string& torrent_file);

        // get MetainfoFile ptr
        const bentypes::MetainfoFile * GetMetainfoFile() const;

        // get the bittorrent task torrent file path
        std::string GetTorrentFile() const;

        // get the BitData info hash of torrent file
        Sha1Value GetInfoHash() const;

        // get the bittorrent task peer_id, the return value is string, store
        // the peer_id, 20 length
        std::string GetPeerId() const;

        // get total uploaded bytes
        long long GetUploaded() const;

        // get total downloaded bytes
        long long GetDownloaded() const;

        // get total size bytes of need download
        long long GetTotalSize() const;

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
        typedef ScopePtr<bentypes::MetainfoFile> MetaInfoPtr;

        // base data
        Sha1Value info_hash_;
        std::string torrent_file_;
        std::string peer_id_;
        long long uploaded_;
        long long downloaded_;

        MetaInfoPtr metainfo_file_;
        ListenInfoSet unused_peers_;
        ListenInfoSet used_peers_;
        PeerDataSet peer_data_set_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_DATA_H

#ifndef BIT_DATA_H
#define BIT_DATA_H

#include "BitPeerData.h"
#include "bencode/MetainfoFile.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../sha1/Sha1Value.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bittorrent {
namespace core {

    // a class store bittorrent task data info
    class BitData : private NotCopyable
    {
    public:
        // peer data pointer typedef
        typedef std::tr1::shared_ptr<BitPeerData> PeerDataPtr;

        // a key class for the peer data
        struct PeerKey
        {
            PeerKey(unsigned long i, unsigned short p)
                : ip(i), port(p)
            {
            }

            friend bool operator < (const PeerKey& left,
                                    const PeerKey& right)
            {
                if (left.ip == right.ip)
                    return left.port < right.port;
                return left.ip < right.ip;
            }

            unsigned long ip;
            unsigned short port;
        };

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

        // add new peer data by peer key, return the peer data pointer
        // if key is exist, then return the exist PeerDataPtr
        PeerDataPtr AddPeerData(const PeerKey& key);

        // get exist peer data pointer, if key is not exist, then return
        // a empty PeerDataPtr
        PeerDataPtr GetPeerData(const PeerKey& key) const;

        // get all peer data pointer
        void GetAllPeerData(std::vector<PeerDataPtr>& peers) const;

    private:
        typedef std::map<PeerKey, PeerDataPtr> PeersData;

        // base data
        std::string torrent_file_;
        Sha1Value info_hash_;
        std::string peer_id_;
        long long uploaded_;
        long long downloaded_;

        // torrent file pointer
        ScopePtr<bentypes::MetainfoFile> metainfo_file_;
        PeersData peers_data_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_DATA_H

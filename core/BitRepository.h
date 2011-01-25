#ifndef BIT_REPOSITORY_H
#define BIT_REPOSITORY_H

#include "../base/BaseTypes.h"
#include "../sha1/Sha1Value.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bittorrent {
namespace core {

    class BitData;

    // a repository store all BitData
    class BitRepository : private NotCopyable
    {
    public:
        typedef std::tr1::shared_ptr<BitData> BitDataPtr;

        // get the BitRepository singleton instance
        static BitRepository& GetSingleton();

        // get current listen port(host)
        short GetListenPort() const;

        // set the bittorrent listen port(host)
        void SetListenPort(short port);

        // create a BitData from torrent_file
        BitDataPtr CreateBitData(const std::string& torrent_file);

        // get all BitDataPtrs
        void GetAllBitData(std::vector<BitDataPtr>& data) const;

    private:
        typedef std::map<Sha1Value, BitDataPtr> BitDataMap;

        BitRepository();
        ~BitRepository();

        short listen_port_;
        BitDataMap bitdata_map_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_REPOSITORY_H

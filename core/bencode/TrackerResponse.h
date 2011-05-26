#ifndef TRACKER_RESPONSE_H
#define TRACKER_RESPONSE_H

#include "../../base/BaseTypes.h"
#include "BenTypes.h"
#include <string>
#include <vector>

namespace bitwave {
namespace core {
namespace bentypes {

    class TrackerResponse : private NotCopyable
    {
    public:
        struct PeerInfo
        {
            unsigned long ip;
            unsigned short port;
        };

        TrackerResponse(const char *data, std::size_t size);

        bool IsFailure() const;
        std::string GetFailureReason() const;
        int GetInterval() const;
        void GetPeerInfo(std::vector<PeerInfo>& peers_info) const;

    private:
        BenTypesStreamBuf response_buffer_;
        std::tr1::shared_ptr<BenType> ben_dic_;
        BenDictionary *response_dic_;
        BenString *failure_reason_;
        BenInteger *interval_;
        BenString *peers_;
    };

} // namespace bentypes
} // namespace core
} // namespace bitwave

#endif // TRACKER_RESPONSE_H

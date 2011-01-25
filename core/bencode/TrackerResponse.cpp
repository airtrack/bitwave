#include "TrackerResponse.h"
#include "../../net/NetHelper.h"

namespace bittorrent {
namespace core {
namespace bentypes {

    TrackerResponse::TrackerResponse(const char *data, std::size_t size)
        : response_buffer_(data, size),
          ben_dic_(GetBenObject(response_buffer_)),
          response_dic_(0),
          failure_reason_(0),
          interval_(0),
          peers_(0)
    {
        response_dic_ = dynamic_cast<BenDictionary *>(ben_dic_.get());
        if (!response_dic_) return ;
        failure_reason_ = response_dic_->ValueBenTypeCast<BenString>("failure reason");
        interval_ = response_dic_->ValueBenTypeCast<BenInteger>("interval");
        peers_ = response_dic_->ValueBenTypeCast<BenString>("peers");
    }

    bool TrackerResponse::IsFailure() const
    {
        return failure_reason_ != 0;
    }

    std::string TrackerResponse::GetFailureReason() const
    {
        if (failure_reason_)
            return failure_reason_->std_string();
        return std::string("not failure");
    }

    int TrackerResponse::GetInterval() const
    {
        if (interval_)
            return interval_->GetValue();
        return 0;
    }

    void TrackerResponse::GetPeerInfo(std::vector<PeerInfo>& peers_info) const
    {
        if (!peers_)
            return ;

        std::size_t number = peers_->length() / 6;
        peers_info.reserve(number);

        const char *data = peers_->data();
        for (std::size_t i = 0; i < number; ++i)
        {
            PeerInfo info;
            info.ip = net::NetToHostl(*reinterpret_cast<const unsigned long *>(data));
            data += 4;
            info.port = net::NetToHosts(*reinterpret_cast<const unsigned short *>(data));
            data += 2;

            peers_info.push_back(info);
        }
    }

} // namespace bentypes
} // namespace core
} // namespace bittorrent

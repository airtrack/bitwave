#ifndef BIT_PEER_LISTENER_H
#define BIT_PEER_LISTENER_H

#include "BitPeerConnection.h"
#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../net/IoService.h"
#include <set>
#include <memory>

namespace bitwave {
namespace core {

    // a class listen all peers connecting
    class BitPeerListener : private NotCopyable
    {
    public:
        explicit BitPeerListener(net::IoService& io_service);

    private:
        class NewPeersHost : public PeerConnectionOwner, private NotCopyable
        {
        public:
            typedef std::tr1::shared_ptr<BitPeerConnection> PeerPtr;
            typedef std::set<PeerPtr> NewPeers;

            void HostingNewPeer(const PeerPtr& peer)
            {
                new_peers_.insert(peer);
            }

        private:
            virtual void LetMeLeave(const std::tr1::shared_ptr<BitPeerConnection>& child)
            {
                new_peers_.erase(child);
            }

            NewPeers new_peers_;
        };

        bool CreateListener();
        void WaitingForPeer();
        void AcceptHandler(bool success, net::BaseSocket peer_sock);

        net::IoService& io_service_;
        ScopePtr<net::AsyncListener> listener_;
        NewPeersHost new_peers_host_;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_PEER_LISTENER_H

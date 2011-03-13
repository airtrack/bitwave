#include "BitPeerListener.h"
#include "BitService.h"
#include "BitRepository.h"
#include <assert.h>
#include <functional>

namespace bittorrent {
namespace core {

    BitPeerListener::BitPeerListener(net::IoService& io_service)
        : io_service_(io_service)
    {
        if (!CreateListener())
            throw CanNotCreatePeerListener();

        WaitingForPeer();
    }

    bool BitPeerListener::CreateListener()
    {
        short hsport = 6881;

        // we try 100 ports to listen
        for (int i = 0; i < 100; ++i)
        {
            net::Address any;
            net::Port port(hsport);

            try
            {
                listener_.Reset(new net::AsyncListener(any, port, io_service_));

                assert(BitService::repository);
                BitService::repository->SetListenPort(hsport);

                // successful, we return
                return true;
            }
            catch (net::NetException&)
            {
                // log NetException here
            }

            ++hsport;
        }
        return false;
    }

    void BitPeerListener::WaitingForPeer()
    {
        listener_->AsyncAccept(
                std::tr1::bind(&BitPeerListener::AcceptHandler, this,
                    std::tr1::placeholders::_1, std::tr1::placeholders::_2));
    }

    void BitPeerListener::AcceptHandler(bool success, net::BaseSocket peer_sock)
    {
        if (success)
        {
            net::AsyncSocket peer = net::MakeAsyncSocket(io_service_, peer_sock);
            NewPeersHost::PeerPtr peer_ptr(new BitPeerConnection(peer, &new_peers_host_));
            new_peers_host_.HostingNewPeer(peer_ptr);
        }

        // waiting next peer
        WaitingForPeer();
    }

} // namespace core
} // namespace bittorrent

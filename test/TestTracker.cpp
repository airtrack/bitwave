#include "../core/BitCreator.h"
#include "../core/BitController.h"
#include "../core/BitData.h"
#include "../core/BitPeerData.h"
#include "../core/BitRepository.h"
#include "../core/BitTask.h"
#include "../net/WinSockIniter.h"
#include "../net/IoService.h"
#include "../net/ResolveService.h"
#include "../net/TimerService.h"
#include <assert.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace bittorrent;
using namespace bittorrent::core;
using namespace bittorrent::net;

std::string ReadableIp(unsigned long ip)
{
    std::ostringstream oss;
    oss << ((ip >> 24) & 0xFF) << '.'
        << ((ip >> 16) & 0xFF) << '.'
        << ((ip >> 8) & 0xFF) << '.'
        << ((ip) & 0xFF);
    return oss.str();
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "usage: TestTracker torrent_file" << std::endl;
        return 0;
    }

    const char *torrent_file = argv[1];
    WinSockIniter sock_initer;
    IoService io_service;
    TimerService timer_service;
    ResolveService resolve_service;
    io_service.AddService(&timer_service);
    io_service.AddService(&resolve_service);

    BitRepository::GetSingleton().SetListenPort(6881);
    BitController bit_controller;
    BitNewTaskCreator bit_creator(bit_controller, io_service);

    if (!bit_creator.CreateTask(torrent_file))
    {
        std::cout << torrent_file << " not exist or not legal." << std::endl;
        return 0;
    }

    for (int i = 0; i < 100; ++i)
    {
        io_service.Run();
        ::Sleep(100);
    }

    std::vector<BitRepository::BitDataPtr> data;
    BitRepository::GetSingleton().GetAllBitData(data);
    assert(data.size() == 1);
    BitRepository::BitDataPtr bitdata = data[0];

    std::vector<BitData::PeerDataPtr> peers_data;
    bitdata->GetAllPeerData(peers_data);

    std::ofstream peers_info_file(std::string(torrent_file) + std::string("_peers_info.txt"));
    for (std::vector<BitData::PeerDataPtr>::iterator it = peers_data.begin();
            it != peers_data.end(); ++it)
    {
        peers_info_file << "ip: " << ReadableIp((*it)->GetIp()) << "\t"
                        << "port: " << (*it)->GetPort() << "\n";
    }

    return 0;
}

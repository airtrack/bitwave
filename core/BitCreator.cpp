#include "BitCreator.h"
#include "BitController.h"
#include "BitTask.h"
#include "BitRepository.h"

namespace bittorrent {
namespace core{

    BitNewTaskCreator::BitNewTaskCreator(BitController& controller,
                                         net::IoService& io_service,
                                         net::ResolveService& resolve_service)
        : controller_(controller),
          io_service_(io_service),
          resolve_service_(resolve_service)
    {
    }

    bool BitNewTaskCreator::CreateTask(const std::string& torrent_file)
    {
        try
        {
            BitRepository::BitDataPtr bitdata =
                BitRepository::GetSingleton().CreateBitData(torrent_file);
            BitTask *task = new BitTask(bitdata,
                    io_service_, resolve_service_);
            controller_.AddTask(BitController::TaskPtr(task));
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

} // namespace core
} // namespace bittorrent

#include "BitCreator.h"
#include "BitController.h"
#include "BitData.h"
#include "BitTask.h"
#include "BitRepository.h"
#include "BitService.h"

namespace bitwave {
namespace core{

    BitNewTaskCreator::BitNewTaskCreator(BitController& controller,
                                         net::IoService& io_service)
        : controller_(controller),
          io_service_(io_service)
    {
    }

    void BitNewTaskCreator::CreateTask(const std::string& torrent_file,
                                       const std::string& download_path)
    {
        BitRepository::BitDataPtr bitdata =
            BitService::repository->CreateBitData(torrent_file);
        bitdata->SelectAllFile(true);

        std::string path = download_path;
        if (path.back() == '\\')
            path.pop_back();
        bitdata->SetBasePath(path);

        BitTask *task = new BitTask(bitdata, io_service_);
        controller_.AddTask(std::tr1::shared_ptr<BitTask>(task));
    }

} // namespace core
} // namespace bitwave

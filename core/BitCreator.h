#ifndef BIT_CREATOR_H
#define BIT_CREATOR_H

#include "../base/BaseTypes.h"
#include "../net/IoService.h"
#include <string>

namespace bittorrent {
namespace core {

    class BitController;

    // this class to create a task and add created task to BitController
    class BitNewTaskCreator : private NotCopyable
    {
    public:
        // construct a creator, all created tasks will add to controller
        BitNewTaskCreator(BitController& controller,
                          net::IoService& io_service);

        // create a new task from a torrent_file and return success or not
        bool CreateTask(const std::string& torrent_file,
                        const std::string& download_path);

    private:
        BitController& controller_;
        net::IoService& io_service_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_CREATOR_H

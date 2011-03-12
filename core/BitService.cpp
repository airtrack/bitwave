#include "BitService.h"

namespace bittorrent {
namespace core {

    net::IoService * BitService::io_service = 0;
    BitController * BitService::controller = 0;
    BitRepository * BitService::repository = 0;
    BitNewTaskCreator * BitService::new_task_creator = 0;

} // namespace core
} // namespace bittorrent

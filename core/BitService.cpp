#include "BitService.h"

namespace bitwave {
namespace core {

    bool BitService::continue_run = false;
    net::IoService * BitService::io_service = 0;
    BitController * BitService::controller = 0;
    BitRepository * BitService::repository = 0;
    BitNewTaskCreator * BitService::new_task_creator = 0;

} // namespace core
} // namespace bitwave

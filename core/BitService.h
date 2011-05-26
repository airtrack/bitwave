#ifndef BIT_SERVICE_H
#define BIT_SERVICE_H

#include "../base/BaseTypes.h"
#include "../net/IoService.h"

namespace bitwave {
namespace core {

    class BitRepository;
    class BitController;
    class BitNewTaskCreator;

    class BitService : private StaticClass
    {
    public:
        static bool continue_run;
        static net::IoService *io_service;
        static BitController *controller;
        static BitRepository *repository;
        static BitNewTaskCreator *new_task_creator;
    };

} // namespace core
} // namespace bitwave

#endif // BIT_SERVICE_H

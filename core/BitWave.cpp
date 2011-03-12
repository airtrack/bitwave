#include "BitWave.h"
#include "BitService.h"
#include "BitCreator.h"
#include "BitController.h"
#include "BitRepository.h"
#include "BitPeerListener.h"
#include <assert.h>
#include <algorithm>

namespace bittorrent {
namespace core {

    BitNetWaveObject::BitNetWaveObject()
        : sock_initer_(),
          io_service_(),
          timer_service_(),
          resolve_service_()
    {
        io_service_.AddService(&timer_service_);
        io_service_.AddService(&resolve_service_);
        BitService::io_service = &io_service_;
    }

    BitNetWaveObject::~BitNetWaveObject()
    {
        BitService::io_service = 0;
    }

    bool BitNetWaveObject::Wave()
    {
        io_service_.Run();
        return true;
    }

    BitCoreControlObject::BitCoreControlObject()
    {
        repository_.Reset(new BitRepository);
        controller_.Reset(new BitController);

        assert(BitService::io_service);
        new_task_creator_.Reset(new BitNewTaskCreator(
                    *controller_, *BitService::io_service));

        BitService::controller = controller_.Get();
        BitService::repository = repository_.Get();
        BitService::new_task_creator = new_task_creator_.Get();

        peer_listener_.Reset(new BitPeerListener(*BitService::io_service));
    }

    BitCoreControlObject::~BitCoreControlObject()
    {
        BitService::controller = 0;
        BitService::repository = 0;
        BitService::new_task_creator = 0;
    }

    void BitWave::AddWaveObject(BitWaveObject *object)
    {
        assert(object);
        wave_objects_.push_back(object);
    }

    void BitWave::RemoveWaveObject(BitWaveObject *object)
    {
        assert(object);
        WaveObjects::iterator it = std::find(
                wave_objects_.begin(), wave_objects_.end(), object);
        if (it != wave_objects_.end())
            wave_objects_.erase(it);
    }

    void BitWave::Wave()
    {
        while (true)
        {
            for (WaveObjects::iterator it = wave_objects_.begin();
                    it != wave_objects_.end(); ++it)
            {
                if (!(*it)->Wave())
                    return ;
            }

            ::Sleep(10);
        }
    }

} // namespace core
} // namespace bittorrent

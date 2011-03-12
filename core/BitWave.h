#ifndef BIT_WAVE_H
#define BIT_WAVE_H

#include "../base/BaseTypes.h"
#include "../base/ScopePtr.h"
#include "../net/WinSockIniter.h"
#include "../net/IoService.h"
#include "../net/TimerService.h"
#include "../net/ResolveService.h"
#include <vector>

namespace bittorrent {
namespace core {

    class BitWaveObject
    {
    public:
        // return true the wave continue, or wave stop
        virtual bool Wave() = 0;
    };

    class BitNetWaveObject : public BitWaveObject, private NotCopyable
    {
    public:
        BitNetWaveObject();
        ~BitNetWaveObject();
        virtual bool Wave();

    private:
        net::WinSockIniter sock_initer_;
        net::IoService io_service_;
        net::TimerService timer_service_;
        net::ResolveService resolve_service_;
    };

    class BitRepository;
    class BitController;
    class BitNewTaskCreator;
    class BitPeerListener;

    class BitCoreControlObject : public BitWaveObject, private NotCopyable
    {
    public:
        BitCoreControlObject();
        ~BitCoreControlObject();
        virtual bool Wave() { return true; }

    private:
        ScopePtr<BitRepository> repository_;
        ScopePtr<BitController> controller_;
        ScopePtr<BitNewTaskCreator> new_task_creator_;
        ScopePtr<BitPeerListener> peer_listener_;
    };

    class BitWave : private NotCopyable
    {
    public:
        void AddWaveObject(BitWaveObject *object);
        void RemoveWaveObject(BitWaveObject *object);
        void Wave();

    private:
        typedef std::vector<BitWaveObject *> WaveObjects;
        WaveObjects wave_objects_;
    };

} // namespace core
} // namespace bittorrent

#endif // BIT_WAVE_H

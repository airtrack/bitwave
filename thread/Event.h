#ifndef EVENT_H
#define EVENT_H

#include "../base/BaseTypes.h"
#include <Windows.h>

namespace bittorrent {

    // a simply Event template wrapper for Windows event.
    template<bool ManualReset>
    class Event : private NotCopyable
    {
    public:
        enum WAIT_RESULT
        {
            SUCCESS,
            TIMEOUT,
            FAILED,
        }

        // default construct an Event without set
        explicit Event(bool init_set = false)
        {
            event_ = ::CreateEvent(0, ManualReset, init_set, 0);
        }

        ~Event()
        {
            ::CloseHandle(event_);
        }

        void SetEvent()
        {
            ::SetEvent(event_);
        }

        void ResetEvent()
        {
            ::ResetEvent(event_);
        }

        WAIT_RESULT Wait(DWORD milliseconds)
        {
            DWORD result = ::WaitForSingleObject(event_, milliseconds);

            switch (result)
            {
            case WAIT_OBJECT_0:
                return SUCCESS;
            case WAIT_TIMEOUT:
                return TIMEOUT;
            default:
                return FAILED;
            }
        }

        bool WaitForever()
        {
            return Wait(INFINITE) == SUCCESS;
        }

    private:
        HANDLE event_;
    };

    typedef Event<true> ManualResetEvent;
    typedef Event<false> AutoResetEvent;

} // namespace bittorrent

#endif // EVENT_H

#ifndef SERVICE_BASE_H
#define SERVICE_BASE_H

#include "../base/BaseTypes.h"
#include <assert.h>

namespace bitwave {
namespace net {

    // all net service's id type
    class ServiceId {};

    // all net service's base class
    class ServiceBase : private NotCopyable
    {
    public:
        ServiceBase()
            : next_service_(0)
        {
        }

        virtual ~ServiceBase()
        {
        }

        void AddService(ServiceBase *service)
        {
            assert(service);
            if (next_service_)
                next_service_->AddService(service);
            else
                next_service_ = service;
        }

        template<typename Service>
        Service * GetService()
        {
            if (&Service::id_ == &GetServiceId())
                return static_cast<Service *>(this);
            else if (next_service_)
                return next_service_->GetService<Service>();
            else
                return 0;
        }

        void Run()
        {
            DoRun();
            if (next_service_)
                next_service_->Run();
        }

    private:
        virtual void DoRun() = 0;
        virtual const ServiceId& GetServiceId() const = 0;

        ServiceBase *next_service_;
    };

    // template Service base class define ServiceId
    template<typename Service>
    class BasicService : public ServiceBase
    {
    public:
        static ServiceId id_;

    private:
        virtual const ServiceId& GetServiceId() const
        {
            return id_;
        }
    };

    template<typename Service>
    ServiceId BasicService<Service>::id_;

    template<typename Service>
    class ServicePtr
    {
    public:
        explicit ServicePtr(ServiceBase& service)
            : service_(0)
        {
            service_ = service.GetService<Service>();
        }

        operator bool () const
        {
            return service_ != 0;
        }

        Service * operator -> () const
        {
            return service_;
        }

        Service& operator * () const
        {
            return *service_;
        }

    private:
        Service *service_;
    };

} // namespace net
} // namespace bitwave

#endif // SERVICE_BASE_H

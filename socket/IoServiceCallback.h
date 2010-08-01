#ifndef _IO_SERVICE_CALLBACK_H_
#define _IO_SERVICE_CALLBACK_H_

#include "Socket.h"
#include "../base/BaseTypes.h"

namespace bittorrent
{
    class IoServiceCallbackException : public BaseException
    {
    public:
        explicit IoServiceCallbackException(const char *w)
            : BaseException(w)
        {
        }
    };

    namespace internal
    {
        class CallbackImplBase
        {
        public:
            virtual void AcceptCallback(AcceptorHandler acceptor, SocketHandler accepted) const
            {
                throw IoServiceCallbackException("io service callback type error!");
            }

            virtual void ConnectCallback(SocketHandler sock) const
            {
                throw IoServiceCallbackException("io service callback type error!");
            }

            virtual void SendRecvCallback(SocketHandler sock, WSABUF buf) const
            {
                throw IoServiceCallbackException("io service callback type error!");
            }

            virtual ~CallbackImplBase() = 0;
        };

        CallbackImplBase::~CallbackImplBase()
        {
        }

        class AcceptCallbackImpl : public CallbackImplBase
        {
        public:
            explicit AcceptCallbackImpl(void (*f)(AcceptorHandler, SocketHandler))
                : callback_(f)
            {
            }

            virtual void AcceptCallback(AcceptorHandler acceptor, SocketHandler accepted) const
            {
                callback_(acceptor, accepted);
            }

        private:
            void (*callback_)(AcceptorHandler, SocketHandler);
        };

        class ConnectCallbackImpl : public CallbackImplBase
        {
        public:
            explicit ConnectCallbackImpl(void (*f)(SocketHandler))
                : callback_(f)
            {
            }

            virtual void ConnectCallback(SocketHandler sock) const
            {
                callback_(sock);
            }

        private:
            void (*callback_)(SocketHandler);
        };

        class SendRecvCallbackImpl : public CallbackImplBase
        {
        public:
            explicit SendRecvCallbackImpl(void (*f)(SocketHandler, WSABUF))
                : callback_(f)
            {
            }

            virtual void SendRecvCallback(SocketHandler sock, WSABUF buf) const
            {
                callback_(sock, buf);
            }

        private:
            void (*callback_)(SocketHandler, WSABUF);
        };
    } // namespace internal

    class IoServiceCallback : private NotCopyable
    {
    public:
        IoServiceCallback()
            : impl_(0)
        {
        }

        ~IoServiceCallback()
        {
            Release();
        }

        explicit IoServiceCallback(void (*f)(SocketHandler))
            : impl_(new internal::ConnectCallbackImpl(f))
        {
        }

        explicit IoServiceCallback(void (*f)(SocketHandler, WSABUF))
            : impl_(new internal::SendRecvCallbackImpl(f))
        {
        }

        explicit IoServiceCallback(void (*f)(AcceptorHandler, SocketHandler))
            : impl_(new internal::AcceptCallbackImpl(f))
        {
        }

        IoServiceCallback& operator = (void (*f)(SocketHandler))
        {
            Release();
            impl_ = new internal::ConnectCallbackImpl(f);
            return *this;
        }

        IoServiceCallback& operator = (void (*f)(SocketHandler, WSABUF))
        {
            Release();
            impl_ = new internal::SendRecvCallbackImpl(f);
            return *this;
        }

        IoServiceCallback& operator = (void (*f)(AcceptorHandler, SocketHandler))
        {
            Release();
            impl_ = new internal::AcceptCallbackImpl(f);
            return *this;
        }

        bool Empty() const
        {
            return impl_ == 0;
        }

        void Release()
        {
            delete impl_;
            impl_ = 0;
        }

        void operator () (SocketHandler sock) const
        {
            impl_->ConnectCallback(sock);
        }

        void operator () (SocketHandler sock, WSABUF buf) const
        {
            impl_->SendRecvCallback(sock, buf);
        }

        void operator () (AcceptorHandler acceptor, SocketHandler accepted) const
        {
            impl_->AcceptCallback(acceptor, accepted);
        }

    private:
        internal::CallbackImplBase *impl_;
    };
} // namespace bittorrent

#endif // _IO_SERVICE_CALLBACK_H_

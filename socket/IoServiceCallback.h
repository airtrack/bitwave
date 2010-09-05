#ifndef _IO_SERVICE_CALLBACK_H_
#define _IO_SERVICE_CALLBACK_H_

#include "Buffer.h"
#include "../base/BaseTypes.h"

namespace bittorrent
{
    class SocketHandler;
    class AcceptorHandler;

    class IoServiceCallbackException : public BaseException
    {
    public:
        explicit IoServiceCallbackException(const char *w)
            : BaseException(w)
        {
        }
    };

    // all callback function prototype
    typedef void (*ConnectCallbackType)(SocketHandler&);
    typedef void (*AcceptCallbackType)(AcceptorHandler&, SocketHandler&);
    typedef void (*SendCallbackType)(SocketHandler&, Buffer& buffer);
    typedef void (*RecvCallbackType)(SocketHandler&, Buffer& buffer, std::size_t);

    namespace internal
    {
        class CallbackImplBase
        {
        public:
            virtual void AcceptCallback(AcceptorHandler& acceptor, SocketHandler& accepted) const
            {
                throw IoServiceCallbackException("io service callback type error!");
            }

            virtual void ConnectCallback(SocketHandler& sock) const
            {
                throw IoServiceCallbackException("io service callback type error!");
            }

            virtual void SendCallback(SocketHandler& sock, Buffer& buffer) const
            {
                throw IoServiceCallbackException("io service callback type error!");
            }

            virtual void RecvCallback(SocketHandler& sock, Buffer& buffer, std::size_t recv) const
            {
                throw IoServiceCallbackException("io service callback type error!");
            }

            virtual ~CallbackImplBase()
            {
            }
        };

        class AcceptCallbackImpl : public CallbackImplBase
        {
        public:
            explicit AcceptCallbackImpl(AcceptCallbackType f)
                : callback_(f)
            {
            }

            virtual void AcceptCallback(AcceptorHandler& acceptor, SocketHandler& accepted) const
            {
                callback_(acceptor, accepted);
            }

        private:
            AcceptCallbackType callback_;
        };

        class ConnectCallbackImpl : public CallbackImplBase
        {
        public:
            explicit ConnectCallbackImpl(ConnectCallbackType f)
                : callback_(f)
            {
            }

            virtual void ConnectCallback(SocketHandler& sock) const
            {
                callback_(sock);
            }

        private:
            ConnectCallbackType callback_;
        };

        class SendCallbackImpl : public CallbackImplBase
        {
        public:
            explicit SendCallbackImpl(SendCallbackType f)
                : callback_(f)
            {
            }

            virtual void SendCallback(SocketHandler& sock, Buffer& buffer) const
            {
                callback_(sock, buffer);
            }

        private:
            SendCallbackType callback_;
        };

        class RecvCallbackImpl : public CallbackImplBase
        {
        public:
            explicit RecvCallbackImpl(RecvCallbackType f)
                : callback_(f)
            {
            }

            virtual void RecvCallback(SocketHandler& sock, Buffer& buffer, std::size_t recv) const
            {
                callback_(sock, buffer, recv);
            }

        private:
            RecvCallbackType callback_;
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
            Reset();
        }

        explicit IoServiceCallback(AcceptCallbackType f)
            : impl_(new internal::AcceptCallbackImpl(f))
        {
        }

        explicit IoServiceCallback(ConnectCallbackType f)
            : impl_(new internal::ConnectCallbackImpl(f))
        {
        }

        explicit IoServiceCallback(SendCallbackType f)
            : impl_(new internal::SendCallbackImpl(f))
        {
        }

        explicit IoServiceCallback(RecvCallbackType f)
            : impl_(new internal::RecvCallbackImpl(f))
        {
        }

        IoServiceCallback& operator = (AcceptCallbackType f)
        {
            Reset();
            impl_ = new internal::AcceptCallbackImpl(f);
            return *this;
        }

        IoServiceCallback& operator = (ConnectCallbackType f)
        {
            Reset();
            impl_ = new internal::ConnectCallbackImpl(f);
            return *this;
        }

        IoServiceCallback& operator = (SendCallbackType f)
        {
            Reset();
            impl_ = new internal::SendCallbackImpl(f);
            return *this;
        }

        IoServiceCallback& operator = (RecvCallbackType f)
        {
            Reset();
            impl_ = new internal::RecvCallbackImpl(f);
            return *this;
        }

        // for any other type T, we just Reset to Empty
        template<typename T>
        IoServiceCallback& operator = (T)
        {
            Reset();
            return *this;
        }

        bool Empty() const
        {
            return impl_ == 0;
        }

        void Reset()
        {
            delete impl_;
            impl_ = 0;
        }

        operator bool () const
        {
            return !Empty();
        }

        void operator () (SocketHandler& sock) const
        {
            impl_->ConnectCallback(sock);
        }

        void operator () (AcceptorHandler& acceptor, SocketHandler& accepted) const
        {
            impl_->AcceptCallback(acceptor, accepted);
        }

        void operator () (SocketHandler& sock, Buffer& buffer) const
        {
            impl_->SendCallback(sock, buffer);
        }

        void operator () (SocketHandler& sock, Buffer& buffer, std::size_t recv) const
        {
            impl_->RecvCallback(sock, buffer, recv);
        }

    private:
        internal::CallbackImplBase *impl_;
    };
} // namespace bittorrent

#endif // _IO_SERVICE_CALLBACK_H_

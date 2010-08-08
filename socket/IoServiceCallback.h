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

    // all callback function prototype
    typedef void (*ConnectCallbackType)(SocketHandler);
    typedef void (*AcceptCallbackType)(AcceptorHandler, SocketHandler);
    typedef void (*SendCallbackType)(SocketHandler, char *, std::size_t);
    typedef void (*RecvCallbackType)(SocketHandler, char *, std::size_t, std::size_t);

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

            virtual void SendCallback(SocketHandler sock, char *data, std::size_t datalen) const
            {
                throw IoServiceCallbackException("io service callback type error!");
            }

            virtual void RecvCallback(SocketHandler sock, char *data, std::size_t datalen, std::size_t recv) const
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
            explicit AcceptCallbackImpl(AcceptCallbackType f)
                : callback_(f)
            {
            }

            virtual void AcceptCallback(AcceptorHandler acceptor, SocketHandler accepted) const
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

            virtual void ConnectCallback(SocketHandler sock) const
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

            virtual void SendCallback(SocketHandler sock, char *data, std::size_t datalen) const
            {
                callback_(sock, data, datalen);
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

            virtual void RecvCallback(SocketHandler sock, char *data, std::size_t datalen, std::size_t recv) const
            {
                callback_(sock, data, datalen, recv);
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
            Release();
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
            Release();
            impl_ = new internal::AcceptCallbackImpl(f);
            return *this;
        }

        IoServiceCallback& operator = (ConnectCallbackType f)
        {
            Release();
            impl_ = new internal::ConnectCallbackImpl(f);
            return *this;
        }

        IoServiceCallback& operator = (SendCallbackType f)
        {
            Release();
            impl_ = new internal::SendCallbackImpl(f);
            return *this;
        }

        IoServiceCallback& operator = (RecvCallbackType f)
        {
            Release();
            impl_ = new internal::RecvCallbackImpl(f);
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

        void operator () (AcceptorHandler acceptor, SocketHandler accepted) const
        {
            impl_->AcceptCallback(acceptor, accepted);
        }

        void operator () (SocketHandler sock, char *data, std::size_t datalen) const
        {
            impl_->SendCallback(sock, data, datalen);
        }

        void operator () (SocketHandler sock, char *data, std::size_t datalen, std::size_t recv) const
        {
            impl_->RecvCallback(sock, data, datalen, recv);
        }

    private:
        internal::CallbackImplBase *impl_;
    };
} // namespace bittorrent

#endif // _IO_SERVICE_CALLBACK_H_

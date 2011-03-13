#ifndef BIT_NET_PROCESSOR_H
#define BIT_NET_PROCESSOR_H

#include "../base/BaseTypes.h"
#include "../buffer/Buffer.h"
#include "../net/IoService.h"
#include "../net/StreamUnpacker.h"
#include <assert.h>
#include <string.h>
#include <functional>

namespace bittorrent {
namespace core {

    template<typename UnpackRuler>
    class BitNetProcessor : private NotCopyable
    {
    public:
        typedef net::StreamUnpacker<UnpackRuler> NetStreamUnpacker;
        typedef typename NetStreamUnpacker::OnUnpackOne ProtocolCallback;
        typedef std::tr1::function<void ()> DisconnectCallback;
        typedef BitNetProcessor<UnpackRuler> ThisType;

        explicit BitNetProcessor(const net::AsyncSocket& socket)
            : connecting_(true),
              send_buffer_count_(0),
              socket_(socket)
        {
            Receive();
        }

        BitNetProcessor(const net::Address& remote_address,
                        const net::Port& remote_listen_port,
                        net::IoService& io_service)
            : connecting_(false),
              send_buffer_count_(0),
              socket_(io_service)
        {
            Connect(remote_address, remote_listen_port);
        }

        ~BitNetProcessor()
        {
            // call destructor must be closed
            assert(!connecting_);
        }

        void SetProtocolCallback(const ProtocolCallback& callback)
        {
            unpacker_.SetUnpackCallback(callback);
        }

        void SetDisconnectCallback(const DisconnectCallback& callback)
        {
            disconnect_callback_ = callback;
        }

        void Send(const char *data, std::size_t size)
        {
            Buffer buffer = GetBuffer(size);
            ::memcpy(buffer.GetBuffer(), data, size);
            Send(buffer);
        }

        void Send(Buffer& buffer)
        {
            socket_.AsyncSend(buffer,
                    std::tr1::bind(&ThisType::SendHandler, this, buffer,
                        std::tr1::placeholders::_1, std::tr1::placeholders::_2));
            ++send_buffer_count_;
        }

        Buffer GetBuffer(std::size_t size) const
        {
            return buffer_cache_.GetBuffer(size);
        }

        void Close()
        {
            socket_.Close();
            if (send_buffer_count_ == 0 && !receive_buffer_)
            {
                connecting_ = false;
                NotifyDisconnect();
            }
        }

    private:
        void Connect(const net::Address& remote_address,
                     const net::Port& remote_listen_port)
        {
            socket_.AsyncConnect(remote_address, remote_listen_port,
                    std::tr1::bind(&ThisType::ConnectHandler,
                        this, std::tr1::placeholders::_1));
            connecting_ = true;
        }

        void ConnectHandler(bool connected)
        {
            if (connected)
                Receive();
            else
                Close();
        }

        void Receive()
        {
            if (!receive_buffer_)
                receive_buffer_ = buffer_cache_.GetBuffer(2048);

            socket_.AsyncReceive(receive_buffer_,
                    std::tr1::bind(&ThisType::ReceiveHandler, this,
                        std::tr1::placeholders::_1, std::tr1::placeholders::_2));
        }

        void ReceiveHandler(bool success, int received)
        {
            if (success)
            {
                unpacker_.StreamDataArrive(receive_buffer_.GetBuffer(), received);
                Receive();
            }
            else
            {
                buffer_cache_.FreeBuffer(receive_buffer_);
                Close();
            }
        }

        void SendHandler(Buffer& buffer, bool success, int send)
        {
            buffer_cache_.FreeBuffer(buffer);
            --send_buffer_count_;

            if (!success)
                Close();
        }

        void NotifyDisconnect()
        {
            assert(disconnect_callback_);
            disconnect_callback_();
        }

        static DefaultBufferCache buffer_cache_;

        bool connecting_;
        int send_buffer_count_;
        Buffer receive_buffer_;
        net::AsyncSocket socket_;
        NetStreamUnpacker unpacker_;
        DisconnectCallback disconnect_callback_;
    };

    template<typename UnpackRuler>
    DefaultBufferCache BitNetProcessor<UnpackRuler>::buffer_cache_;

} // namespace core
} // namespace bittorrent

#endif // BIT_NET_PROCESSOR_H

#ifndef BIT_NET_PROCESSOR_H
#define BIT_NET_PROCESSOR_H

#include "BitService.h"
#include "../base/BaseTypes.h"
#include "../buffer/Buffer.h"
#include "../net/IoService.h"
#include "../net/StreamUnpacker.h"
#include "../net/NetHelper.h"
#include <assert.h>
#include <string.h>
#include <memory>
#include <functional>

namespace bitwave {
namespace core {

    template<typename UnpackRuler, typename ConnectionType>
    class BitNetProcessor : public net::StreamUnpacker<UnpackRuler>,
                            public std::tr1::enable_shared_from_this<
                                BitNetProcessor<UnpackRuler, ConnectionType>>
    {
    public:
        typedef BitNetProcessor<UnpackRuler, ConnectionType> ThisType;
        static const std::size_t receive_buffer_size = 2048;

        // construct a new net processor from an exist socket, then call one
        // time Receive, all other things will be done automatically
        BitNetProcessor(const net::AsyncSocket& socket,
                        ConnectionType *connection)
            : connecting_(true),
              send_buffer_count_(0),
              socket_(socket),
              connection_(connection)
        {
        }

        // construct a new net processor, then call one time Connect, all other
        // things will be done automatically
        BitNetProcessor(net::IoService& io_service,
                        ConnectionType *connection)
            : connecting_(false),
              send_buffer_count_(0),
              socket_(io_service),
              connection_(connection)
        {
        }

        void Connect(const net::Address& remote_address,
                     const net::Port& remote_listen_port)
        {
            try
            {
                socket_.AsyncConnect(remote_address, remote_listen_port,
                        std::tr1::bind(&ThisType::ConnectHandler,
                            shared_from_this(), std::tr1::placeholders::_1));
            }
            catch (const net::NetException&)
            {
                Close();
            }
        }

        void Receive()
        {
            if (!connecting_)
                return ;

            if (!BitService::continue_run)
            {
                unsigned long pending_size =
                    net::GetPendingDataSize(socket_.GetImplement());
                if (pending_size > receive_buffer_size)
                    BitService::continue_run = true;
            }

            if (!receive_buffer_)
                receive_buffer_ = buffer_cache_.GetBuffer(receive_buffer_size);

            try
            {
                socket_.AsyncReceive(receive_buffer_,
                        std::tr1::bind(&ThisType::ReceiveHandler, shared_from_this(),
                            std::tr1::placeholders::_1, std::tr1::placeholders::_2));
            }
            catch (const net::NetException&)
            {
                buffer_cache_.FreeBuffer(receive_buffer_);
                Close();
            }
        }

        void Send(const char *data, std::size_t size)
        {
            Buffer buffer = GetBuffer(size);
            memcpy(buffer.GetBuffer(), data, size);
            Send(buffer);
        }

        void Send(Buffer& buffer)
        {
            if (connecting_)
            {
                try
                {
                    socket_.AsyncSend(buffer,
                            std::tr1::bind(&ThisType::SendHandler, shared_from_this(),
                                buffer, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
                    ++send_buffer_count_;
                }
                catch (const net::NetException&)
                {
                    buffer_cache_.FreeBuffer(buffer);
                    Close();
                }
            }
            else
            {
                // we free the buffer silently
                buffer_cache_.FreeBuffer(buffer);
            }
        }

        Buffer GetBuffer(std::size_t size) const
        {
            return buffer_cache_.GetBuffer(size);
        }

        void Close()
        {
            socket_.Close();
            connecting_ = false;
            if (send_buffer_count_ == 0 && !receive_buffer_)
                OnDisconnect();
        }

        void ClearConnection()
        {
            connection_ = 0;
        }

        bool Connecting() const
        {
            return connecting_;
        }

    private:
        virtual void OnUnpackOne(const char *data, std::size_t size)
        {
            if (connection_)
                connection_->ProcessProtocol(data, size);
        }

        void OnConnect()
        {
            if (connection_)
                connection_->OnConnect();
        }

        void OnDisconnect()
        {
            if (connection_)
                connection_->OnDisconnect();
        }

        void ConnectHandler(bool connected)
        {
            if (connected)
            {
                connecting_ = true;
                OnConnect();
                Receive();
            }
            else
            {
                Close();
            }
        }

        void ReceiveHandler(bool success, int received)
        {
            if (success)
            {
                Buffer temp = receive_buffer_;
                receive_buffer_.Reset();
                Receive();
                StreamDataArrive(temp.GetBuffer(), received);
                buffer_cache_.FreeBuffer(temp);
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

        static DefaultBufferCache buffer_cache_;

        bool connecting_;
        int send_buffer_count_;
        Buffer receive_buffer_;
        net::AsyncSocket socket_;
        ConnectionType *connection_;
    };

    template<typename UnpackRuler, typename ConnectionType>
    DefaultBufferCache BitNetProcessor<UnpackRuler, ConnectionType>::buffer_cache_;

} // namespace core
} // namespace bitwave

#endif // BIT_NET_PROCESSOR_H

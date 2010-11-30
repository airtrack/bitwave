#ifndef BUFFER_H
#define BUFFER_H

#include "../base/BaseTypes.h"
#include <assert.h>
#include <algorithm>
#include <map>

namespace bittorrent {

    namespace internal {

        class Chunk
        {
        public:
            Chunk(std::size_t buffersize, std::size_t buffernum)
                : buffers_(0),
                  first_(0),
                  available_(0),
                  maxnum_(0),
                  buffersize_(0)
            {
                assert(buffersize > sizeof(char *) && buffernum > 0);
                std::size_t len = buffersize * buffernum;
                buffers_ = new char[len];
                first_ = buffers_;
                available_ = buffernum;
                maxnum_ = buffernum;
                buffersize_ = buffersize;

                char *begin = buffers_;
                char *end = buffers_ + len - buffersize_;
                *reinterpret_cast<char **>(end) = 0;
                for (; begin < end; begin += buffersize_)
                {
                    char **next = reinterpret_cast<char **>(begin);
                    *next = begin + buffersize_;
                }
            }

            ~Chunk()
            {
                delete [] buffers_;
            }

            char * Allocate()
            {
                if (!first_)
                    return 0;
                char *result = first_;
                first_ = *reinterpret_cast<char **>(first_);
                return result;
            }

            void Deallocate(char *buffer)
            {
                *reinterpret_cast<char **>(buffer) = first_;
                first_ = buffer;
            }

            bool IsFull() const
            {
                return available_ == 0;
            }

            // the buffer is one of this chunk
            bool IsChunkBuffer(char *buffer) const
            {
                assert(buffer);
                return buffer >= buffers_ && buffer < buffers_ + maxnum_ * buffersize_;
            }

        private:
            char *buffers_;
            char *first_;
            std::size_t available_;
            std::size_t maxnum_;
            std::size_t buffersize_;
        };

    } // namespace internal

    template<typename BufferSizePolicy>
    class FixedBufferAllocator
    {
        typedef std::multimap<size_t, internal::Chunk *> BufferPool;
        typedef std::pair<BufferPool::iterator, BufferPool::iterator> BufferIterPair;

    public:
        ~FixedBufferAllocator()
        {
            std::for_each(bufferpool_.begin(), bufferpool_.end(), DeleteSecondOfPair());
        }

        char * Allocate(std::size_t size)
        {
            assert(size > 0);
            if (BufferSizePolicy::BigThanMax(size))
                return new char[size];

            std::size_t fixsize = BufferSizePolicy::GetUpBoundSize(size);
            BufferIterPair equalrange = bufferpool_.equal_range(fixsize);

            for (; equalrange.first != equalrange.second; ++equalrange.first)
            {
                if (!equalrange.first->second->IsFull())
                    return equalrange.first->second->Allocate();
            }

            BufferPool::iterator itnew = bufferpool_.insert(std::make_pair(fixsize,
                new internal::Chunk(fixsize, BufferSizePolicy::GetNumPerChunk(fixsize))));
            return itnew->second->Allocate();
        }

        void Deallocate(char *buffer, std::size_t size)
        {
            assert(buffer && size > 0);
            if (BufferSizePolicy::BigThanMax(size))
                return delete [] buffer;

            std::size_t fixsize = BufferSizePolicy::GetUpBoundSize(size);
            BufferIterPair equalrange = bufferpool_.equal_range(fixsize);

            for (; equalrange.first != equalrange.second; ++equalrange.first)
            {
                if (equalrange.first->second->IsChunkBuffer(buffer))
                {
                    equalrange.first->second->Deallocate(buffer);
                    return ;
                }
            }

            // should not get here
            assert(0);
        }

    private:
        BufferPool bufferpool_;
    };

    struct DefaultBufferSizePolicy
    {
        static const std::size_t MaxBufferSize = 256 * 1024;
        static const std::size_t MaxNumPerChunk = 128;
        static const std::size_t AlignSize = 8;

        static std::size_t GetUpBoundSize(std::size_t size)
        {
            if (size % AlignSize)
                return ((size / AlignSize) + 1) * AlignSize;
            else
                return size;
        }

        static std::size_t GetNumPerChunk(std::size_t size)
        {
            std::size_t num = MaxBufferSize / size;
            if (num > MaxNumPerChunk) num = MaxNumPerChunk;
            return num;
        }

        static bool BigThanMax(std::size_t size)
        {
            return size > MaxBufferSize;
        }
    };

    class Buffer
    {
    public:
        Buffer()
            : data_(0),
              len_(0)
        {
        }

        Buffer(char *data, std::size_t len)
            : data_(data),
              len_(len)
        {
        }

        char *GetBuffer() const
        {
            return data_;
        }

        std::size_t BufferLen() const
        {
            return len_;
        }

        void Reset()
        {
            data_ = 0;
            len_ = 0;
        }

        operator bool () const
        {
            return data_ != 0;
        }

    private:
        char *data_;
        std::size_t len_;
    };

    template<typename BufferSizePolicy>
    class BufferCache
    {
    public:
        Buffer GetBuffer(std::size_t size)
        {
            return Buffer(allocator_.Allocate(size), size);
        }

        void FreeBuffer(Buffer& buf)
        {
            allocator_.Deallocate(buf.Get(), buf.Len());
            buf.Reset();
        }

        void FreeBuffer(char *data, std::size_t size)
        {
            allocator_.Deallocate(data, size);
        }

    private:
        FixedBufferAllocator<BufferSizePolicy> allocator_;
    };

    typedef BufferCache<DefaultBufferSizePolicy> DefaultBufferCache;

} // namespace bittorrent

#endif // BUFFER_H

#ifndef _SOCKET_STREAM_BUF_H_
#define _SOCKET_STREAM_BUF_H_

#include "../base/BaseTypes.h"

#include <algorithm>
#include <assert.h>
#include <string.h>

namespace bittorrent
{
    std::size_t default_streambuf_size = 2048;

    template<typename T>
    struct CharTypeTrait
    {
        typedef std::size_t size_type;
        typedef long int_type;
    };

    // this socket stream buffer implement a ring buffer
    template<typename CharType, typename TypeTrait = CharTypeTrait<CharType> >
    class basic_socketstreambuf : private NotCopyable
    {
    public:
        typedef CharType char_type;
        typedef typename TypeTrait::size_type size_type;
        typedef typename TypeTrait::int_type int_type;

        explicit basic_socketstreambuf(size_type size = default_streambuf_size)
            : data_(new char_type[size]),
              size_(size),
              pptr_(data_),
              gptr_(data_),
              hasdata_(false)
        {
        }

        ~basic_socketstreambuf()
        {
            delete [] data_;
        }

        char_type *bufbegin() const
        {
            return data_;
        }

        char_type *bufend() const
        {
            return data_ + size_;
        }

        // put size of char_type to buffer, return real size putted in
        size_type putn(const char_type *data, size_type size)
        {
            assert(data && size > 0);
            size_type retval = 0;
            if (pptr_ == gptr_)
            {
                if (hasdata_)
                {
                    return retval;
                }
                else
                {
                    retval = 1;
                    *pptr_ = *data;
                    if (++pptr_ == bufend())
                        pptr_ = bufbegin();
                }
            }

            if (pptr_ > gptr_)
            {
                size_type s1 = std::min(static_cast<size_type>(bufend() - pptr_), size - retval);
                memcpy(pptr_, data + retval, sizeof(char_type) * s1);
                pptr_ += s1;
                retval += s1;

                if (pptr_ == bufend())
                    pptr_ = bufbegin();
            }

            if (pptr_ < gptr_)
            {
                size_type s2 = std::min(static_cast<size_type>(gptr_ - pptr_), size - retval);
                memcpy(pptr_, data + retval, sizeof(char_type) * s2);
                pptr_ += s2;
                retval += s2;
            }

            hasdata_ = true;
            return retval;
        }

        // get size number of char_type to data, return real size of getted
        size_type getn(char_type *data, size_type size)
        {
            assert(data && size > 0);
            size_type retval = 0;
            if (gptr_ == pptr_)
            {
                if (hasdata_)
                {
                    retval = 1;
                    *data = *gptr_;
                    if (++gptr_ == bufend())
                        gptr_ = bufbegin();
                }
                else
                {
                    return retval;
                }
            }

            if (gptr_ > pptr_)
            {
                size_type s1 = std::min(static_cast<size_type>(bufend() - gptr_), size - retval);
                memcpy(data + retval, gptr_, sizeof(char_type) * s1);
                gptr_ += s1;
                retval += s1;

                if (gptr_ == bufend())
                    gptr_ = bufbegin();
            }

            if (gptr_ < pptr_)
            {
                size_type s2 = std::min(static_cast<size_type>(pptr_ - gptr_), size - retval);
                memcpy(data + retval, gptr_, sizeof(char_type) * s2);
                gptr_ += s2;
                retval += s2;
            }

            if (gptr_ == pptr_)
                hasdata_ = false;
            return retval;
        }

        // peek the first buffered char_type if has data, otherwise return EOF
        int_type peek() const
        {
            if (hasdata_)
                return *gptr_;
            else
                static_cast<int_type>(-1);
        }

        // peek n number of buffered char_type, return real size of peeked data
        size_type peekn(char_type *peeked, size_type n)
        {
            assert(peeked && n > 0);

            // save gptr_ and hasdata_
            char_type *gptr = gptr_;
            bool hasdata = hasdata_;

            size_type retval = getn(peeked, n);

            // restore gptr_ and hasdata_
            gptr_ = gptr;
            hasdata_ = hasdata;

            return retval;
        }

        // data buffered size
        size_type buffered() const
        {
            if (gptr_ < pptr_)
            {
                return pptr_ - gptr_;
            }
            else if (gptr_ > pptr_)
            {
                return size_ - (gptr_ - pptr_);
            }
            else
            {
                if (hasdata_)
                    return size_;
                else
                    return 0;
            }
        }

        // reserve buffer size
        size_type reserved() const
        {
            return size_ - buffered();
        }

        // no data in buffer, means EOF
        bool eof() const
        {
            return !hasdata_;
        }

        // no reserved size
        bool full() const
        {
            return hasdata_ && (gptr_ == pptr_);
        }

    private:
        // pointer to data buffer and its size
        char_type *data_;
        size_type size_;

        // put data pointer
        char_type *pptr_;

        // get data pointer
        char_type *gptr_;

        // a flag about has data in buffer or not
        bool hasdata_;
    };
} // bittorrent

#endif // _SOCKET_STREAM_BUF_H_

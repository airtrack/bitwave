#include "BenTypes.h"
#include <stdlib.h>

namespace bentypes
{
    // BenTypesStreamBuf --------------------------------------------
    BenTypesStreamBuf::BenTypesStreamBuf(const char *buf, std::size_t size)
        : streambuf_(), current_()
    {
        streambuf_.resize(size);
        memcpy(&streambuf_[0], buf, size);
        current_ = streambuf_.begin();
    }

    BenTypesStreamBuf::BenTypesStreamBuf(FILE *file, std::size_t size)
        : streambuf_(), current_()
    {
        streambuf_.resize(size);
        fread(&streambuf_[0], 1, size, file);
        current_ = streambuf_.begin();
    }

    // BenString ----------------------------------------------------
    BenString::BenString(BenTypesStreamBuf& buf)
        : benstr_()
    {
        int stringlen = ReadStringLen(buf);
        if (stringlen <= 0)
            throw BaseException("Error in Construct BenString: string len is <= 0");
        if (buf.IsEOF() || buf.Peek() != ':')
            throw BaseException("Error in Construct BenString: has no ':'");

        buf.Next();
        ReadString(buf, stringlen);
    }

    int BenString::ReadStringLen(BenTypesStreamBuf& buf)
    {
        std::string lenbuf;
        while (!buf.IsEOF() && isdigit(buf.Peek()))
        {
            lenbuf.push_back(buf.Peek());
            buf.Next();
        }

        return atoi(lenbuf.c_str());
    }

    void BenString::ReadString(BenTypesStreamBuf& buf, int len)
    {
        benstr_.reserve(len);
        while (!buf.IsEOF() && len-- > 0)
        {
            benstr_.push_back(buf.Peek());
            buf.Next();
        }
    }

    // BenInteger ---------------------------------------------------
    BenInteger::BenInteger(BenTypesStreamBuf& buf)
        : benint_(0)
    {
        if (buf.IsEOF() || buf.Peek() != 'i')
            throw BaseException("Error in Construct BenInteger: error head, has no 'i'.");
        buf.Next();

        std::string intbuf;
        while (!buf.IsEOF() && buf.Peek() != 'e')
        {
            intbuf.push_back(buf.Peek());
            buf.Next();
        }

        if (buf.IsEOF() || buf.Peek() != 'e')
            throw BaseException("Error in Construct BenInteger: error tail, has no 'e'.");
        buf.Next();

        benint_ = atoi(intbuf.c_str());
    }
} // namespace bentypes
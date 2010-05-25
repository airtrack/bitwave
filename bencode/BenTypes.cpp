#include "BenTypes.h"
#include <stdlib.h>

namespace bittorrent
{

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

        // BenList ------------------------------------------------------
        BenList::BenList(BenTypesStreamBuf& buf)
            : benlist_()
        {
            if (buf.IsEOF() || buf.Peek() != 'l')
                throw BaseException("Error in Construct BenList: error head, has no 'l'.");
            buf.Next();

            while (!buf.IsEOF() && buf.Peek() != 'e')
            {
                benlist_.push_back(GetBenObject(buf));
            }

            if (buf.IsEOF() || buf.Peek() != 'e')
                throw BaseException("Error in Construct BenList: error tail, has no 'e'.");
            buf.Next();
        }

        // BenDictionary ------------------------------------------------
        BenDictionary::BenDictionary(BenTypesStreamBuf& buf)
            : benmap_()
        {
            if (buf.IsEOF() || buf.Peek() != 'd')
                throw BaseException("Error in Construct BenDictionary: error head, has no 'd'.");
            buf.Next();

            while (!buf.IsEOF() && buf.Peek() != 'e')
            {
                BenString key(buf);
                std::tr1::shared_ptr<BenType> value = GetBenObject(buf);
                if (value)
                {
                    benmap_.insert(BenMap::value_type(key.std_string(), value));
                }
            }

            if (buf.IsEOF() || buf.Peek() != 'e')
                throw BaseException("Error in Construct BenDictionary: error tail, has no 'e'.");
            buf.Next();
        }

        std::tr1::shared_ptr<BenType> GetBenObject(BenTypesStreamBuf& buf)
        {
            if (buf.IsEOF())
                return std::tr1::shared_ptr<BenType>();

            switch (buf.Peek())
            {
            case 'i':
                return std::tr1::shared_ptr<BenType>(new BenInteger(buf));

            case 'l':
                return std::tr1::shared_ptr<BenType>(new BenList(buf));

            case 'd':
                return std::tr1::shared_ptr<BenType>(new BenDictionary(buf));

            default:
                return std::tr1::shared_ptr<BenType>(new BenString(buf));
            }
        }
    } // namespace bentypes

} // namespace bittorrent

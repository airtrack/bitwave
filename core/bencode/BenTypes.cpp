#include "BenTypes.h"
#include "../BitException.h"
#include "../../base/StringConv.h"
#include <assert.h>
#include <stdlib.h>
#include <fstream>

namespace bitwave {
namespace core {
namespace bentypes {

    // BenTypesStreamBuf --------------------------------------------
    BenTypesStreamBuf::BenTypesStreamBuf(const char *buf, std::size_t size)
        : streambuf_()
    {
        assert(buf && size > 0);
        streambuf_.resize(size);
        memcpy(&streambuf_[0], buf, size);
    }

    BenTypesStreamBuf::BenTypesStreamBuf(const char *filename)
        : streambuf_()
    {
        assert(filename);
        std::wstring path = UTF8ToUnicode(filename);
        std::ifstream fs(path.c_str(), std::ios_base::in | std::ios_base::binary);

        if (fs.is_open())
        {
            fs.seekg(0, std::ios_base::end);
            std::size_t size = static_cast<std::size_t>(fs.tellg());
            fs.seekg(0, std::ios_base::beg);

            if (size > 0)
            {
                streambuf_.resize(size);
                fs.read(&streambuf_[0], size);
            }
        }
    }

    // BenString ----------------------------------------------------
    BenString::BenString(
            BenTypesStreamBuf::const_iterator& begin,
            BenTypesStreamBuf::const_iterator& end)
        : benstr_()
    {
        srcbufbegin_ = begin;
        int stringlen = ReadStringLen(begin, end);
        if (stringlen <= 0 || begin == end || *begin != ':')
            throw BenTypeException(INVALIDATE_STRING);

        ++begin;
        ReadString(begin, end, stringlen);
        srcbufend_ = begin;
    }

    int BenString::ReadStringLen(
            BenTypesStreamBuf::const_iterator& begin,
            BenTypesStreamBuf::const_iterator& end)
    {
        std::string lenbuf;
        while (begin != end && isdigit(*begin))
        {
            lenbuf.push_back(*begin++);
        }

        return atoi(lenbuf.c_str());
    }

    void BenString::ReadString(
            BenTypesStreamBuf::const_iterator& begin,
            BenTypesStreamBuf::const_iterator& end, int len)
    {
        BenTypesStreamBuf::const_iterator send = begin + len;
        if (send > end)
            throw BenTypeException(INVALIDATE_STRING);

        benstr_.assign(begin, send);
        begin = send;
    }

    // BenInteger ---------------------------------------------------
    BenInteger::BenInteger(
            BenTypesStreamBuf::const_iterator& begin,
            BenTypesStreamBuf::const_iterator& end)
        : benint_(0)
    {
        srcbufbegin_ = begin;
        if (begin == end || *begin != 'i')
            throw BenTypeException(INVALIDATE_INTERGER);
        ++begin;

        std::string intbuf;
        while (begin != end && *begin != 'e')
        {
            intbuf.push_back(*begin++);
        }

        // *begin must be 'e' if it is validate BenInteger
        if (begin == end)
            throw BenTypeException(INVALIDATE_INTERGER);
        ++begin;

        benint_ = _atoi64(intbuf.c_str());
        srcbufend_ = begin;
    }

    // BenList ------------------------------------------------------
    BenList::BenList(
            BenTypesStreamBuf::const_iterator& begin,
            BenTypesStreamBuf::const_iterator& end)
        : benlist_()
    {
        srcbufbegin_ = begin;
        if (begin == end || *begin != 'l')
            throw BenTypeException(INVALIDATE_LIST);
        ++begin;

        while (begin != end && *begin != 'e')
        {
            benlist_.push_back(GetBenObject(begin, end));
        }

        // *begin must be 'e' if it is validate BenList
        if (begin == end)
            throw BenTypeException(INVALIDATE_LIST);
        ++begin;
        srcbufend_ = begin;
    }

    // BenDictionary ------------------------------------------------
    BenDictionary::BenDictionary(
            BenTypesStreamBuf::const_iterator& begin,
            BenTypesStreamBuf::const_iterator& end)
        : benmap_()
    {
        srcbufbegin_ = begin;
        if (begin == end || *begin != 'd')
            throw BenTypeException(INVALIDATE_DICTIONARY);
        ++begin;

        while (begin != end && *begin != 'e')
        {
            BenString key(begin, end);
            std::tr1::shared_ptr<BenType> value = GetBenObject(begin, end);
            if (value)
            {
                benmap_.insert(BenMap::value_type(key.std_string(), value));
            }
        }

        // *begin must be 'e' if it is validate BenDictionary
        if (begin == end)
            throw BenTypeException(INVALIDATE_DICTIONARY);
        ++begin;
        srcbufend_ = begin;
    }

    std::tr1::shared_ptr<BenType> GetBenObject(
            BenTypesStreamBuf::const_iterator& begin,
            BenTypesStreamBuf::const_iterator& end)
    {
        if (begin == end)
            throw BenTypeException(INVALIDATE_NOBENTYPE);

        switch (*begin)
        {
        case 'i':
            return std::tr1::shared_ptr<BenType>(new BenInteger(begin, end));

        case 'l':
            return std::tr1::shared_ptr<BenType>(new BenList(begin, end));

        case 'd':
            return std::tr1::shared_ptr<BenType>(new BenDictionary(begin, end));

        default:
            return std::tr1::shared_ptr<BenType>(new BenString(begin, end));
        }
    }

} // namespace bentypes
} // namespace core
} // namespace bitwave

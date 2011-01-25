#ifndef BEN_TYPES_H
#define BEN_TYPES_H

#include <assert.h>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../../base/BaseTypes.h"

namespace bittorrent {
namespace core {
namespace bentypes {

    // bentypes exception enum code
    enum BenTypeExceptionCode
    {
        INVALIDATE_NOBENTYPE,
        INVALIDATE_INTERGER,
        INVALIDATE_STRING,
        INVALIDATE_LIST,
        INVALIDATE_DICTIONARY,
    };

    // bentypes exception
    struct BenTypeException
    {
        BenTypeException(BenTypeExceptionCode bec)
            : code(bec)
        {}

        BenTypeExceptionCode code;
    };

    // bentypes stream buffer, store raw buffer of bentypes
    class BenTypesStreamBuf : private NotCopyable
    {
    public:
        typedef std::vector<char> buffer_type;
        typedef buffer_type::iterator iterator;
        typedef buffer_type::const_iterator const_iterator;

        BenTypesStreamBuf(const char *buf, std::size_t size);
        BenTypesStreamBuf(const char *filename);

        iterator begin() { return streambuf_.begin(); }
        iterator end() { return streambuf_.end(); }
        const_iterator begin() const { return streambuf_.begin(); }
        const_iterator end() const { return streambuf_.end(); }
        std::size_t size() const { return streambuf_.size(); }
        const char *iter_data(const_iterator it) const
        {
            assert(begin() <= it && it < end());
            return &streambuf_[it - begin()];
        }

    private:
        std::vector<char> streambuf_;
    };

    class BenType
    {
    public:
        BenTypesStreamBuf::const_iterator GetSrcBufBegin() const
        {
            return srcbufbegin_;
        }

        BenTypesStreamBuf::const_iterator GetSrcBufEnd() const
        {
            return srcbufend_;
        }

        virtual ~BenType() { }

    protected:
        BenTypesStreamBuf::const_iterator srcbufbegin_;
        BenTypesStreamBuf::const_iterator srcbufend_;
    };

    class BenString : public BenType
    {
    public:
        explicit BenString(
                BenTypesStreamBuf::const_iterator& begin,
                BenTypesStreamBuf::const_iterator& end);

        std::size_t length() const { return benstr_.size(); }
        const char * c_str() const { return benstr_.c_str(); }
        const char * data() const { return benstr_.data(); }
        const std::string& std_string() const { return benstr_; }
        std::string std_string() { return benstr_; }

    private:
        int ReadStringLen(
                BenTypesStreamBuf::const_iterator& begin,
                BenTypesStreamBuf::const_iterator& end);
        void ReadString(
                BenTypesStreamBuf::const_iterator& begin,
                BenTypesStreamBuf::const_iterator& end, int len);

        std::string benstr_;
    };

    class BenInteger : public BenType
    {
    public:
        explicit BenInteger(
                BenTypesStreamBuf::const_iterator& begin,
                BenTypesStreamBuf::const_iterator& end);

        int GetValue() const { return benint_; }

    private:
        int benint_;
    };

    class BenList : public BenType, private NotCopyable
    {
    public:
        typedef std::list<std::tr1::shared_ptr<BenType> > ListBenTypes;
        typedef ListBenTypes::iterator iterator;
        typedef ListBenTypes::const_iterator const_iterator;
        typedef ListBenTypes::value_type value_type;

        explicit BenList(
                BenTypesStreamBuf::const_iterator& begin,
                BenTypesStreamBuf::const_iterator& end);

        std::size_t size() const { return benlist_.size(); }

        iterator begin() { return benlist_.begin(); }
        const_iterator begin() const { return benlist_.begin(); }

        iterator end() { return benlist_.end(); }
        const_iterator end() const { return benlist_.end(); }

        template<typename T>
        void AllElementPtr(std::vector<T *> *vt) const
        {
            assert(vt);
            vt->reserve(vt->size() + benlist_.size());
            for (ListBenTypes::const_iterator it = benlist_.begin();
                it != benlist_.end(); ++it)
            {
                T *t = dynamic_cast<T *>(it->get());
                if (t) vt->push_back(t);
            }
        }

    private:
        ListBenTypes benlist_;
    };

    class BenDictionary : public BenType, private NotCopyable
    {
    public:
        typedef std::map<std::string, std::tr1::shared_ptr<BenType> > BenMap;
        typedef std::string key_type;
        typedef BenMap::mapped_type value_type;
        typedef BenMap::value_type pair_type;
        typedef BenMap::iterator iterator;
        typedef BenMap::const_iterator const_iterator;

        explicit BenDictionary(
                BenTypesStreamBuf::const_iterator& begin,
                BenTypesStreamBuf::const_iterator& end);

        std::size_t size() const { return benmap_.size(); }

        iterator begin() { return benmap_.begin(); }
        const_iterator begin() const { return benmap_.begin(); }

        iterator end() { return benmap_.end(); }
        const_iterator end() const { return benmap_.end(); }

        iterator find(const std::string& key) { return benmap_.find(key); }
        const_iterator find(const std::string& key) const { return benmap_.find(key); }

        // return a empty value_type object if can not find key
        value_type operator [] (const std::string& key) const
        {
            const_iterator it = find(key);
            if (it == benmap_.end()) return value_type();
            return it->second;
        }

        // get casted value BenType pointer of key, return 0
        // if can not cast or not exist the key
        template<typename T>
        T * ValueBenTypeCast(const std::string& key) const
        {
            T *casted = 0;
            value_type vt = (*this)[key];
            if (vt) casted = dynamic_cast<T *>(vt.get());
            return casted;
        }

    private:
        BenMap benmap_;
    };

    std::tr1::shared_ptr<BenType> GetBenObject(
            BenTypesStreamBuf::const_iterator& begin,
            BenTypesStreamBuf::const_iterator& end);

    inline std::tr1::shared_ptr<BenType> GetBenObject(
            const BenTypesStreamBuf& benstreambuf)
    {
        BenTypesStreamBuf::const_iterator begin = benstreambuf.begin();
        BenTypesStreamBuf::const_iterator end = benstreambuf.end();
        return GetBenObject(begin, end);
    }

} // namespace bentypes
} // namespace core
} // namespace bittorrent

#endif // BEN_TYPES_H

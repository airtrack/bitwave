#ifndef _BEN_TYPES_H_
#define _BEN_TYPES_H_

#include <string>
#include <vector>
#include <map>
#include <list>
#include <memory>
#include <stdio.h>

#include "../base/BaseTypes.h"

namespace bentypes
{
    class BenTypesStreamBuf : private NotCopyable
    {
    public:
        BenTypesStreamBuf(const char *buf, std::size_t size);
        BenTypesStreamBuf(FILE *file, std::size_t size);

        char Peek() const { return *current_; }
        void Next() { ++current_; }
        bool IsEOF() const { return current_ == streambuf_.end(); }
        void Reset() { current_ = streambuf_.begin(); }

    private:
        std::vector<char> streambuf_;
        std::vector<char>::iterator current_;
    };

    class BenType
    {
    public:
        virtual ~BenType() { }
    };

    class BenString : public BenType
    {
    public:
        explicit BenString(BenTypesStreamBuf& buf);

        std::size_t length() const { return benstr_.size(); }
        const char * c_str() const { return benstr_.c_str(); }
        std::string std_string() const { return benstr_; }

    private:
        int ReadStringLen(BenTypesStreamBuf& buf);
        void ReadString(BenTypesStreamBuf& buf, int len);

        std::string benstr_;
    };

    class BenInteger : public BenType
    {
    public:
        explicit BenInteger(BenTypesStreamBuf& buf);
        int GetValue() const { return benint_; }

    private:
        int benint_;
    };

    class BenList : public BenType, private NotCopyable
    {
        typedef std::list<std::tr1::shared_ptr<BenType> > ListBenTypes;
    public:
        typedef ListBenTypes::iterator iterator;
        typedef ListBenTypes::const_iterator const_iterator;
        typedef ListBenTypes::value_type value_type;

        explicit BenList(BenTypesStreamBuf& buf);

        std::size_t size() const { return benlist_.size(); }

        iterator begin() { return benlist_.begin(); }
        const_iterator begin() const { return benlist_.begin(); }

        iterator end() { return benlist_.end(); }
        const_iterator end() const { return benlist_.end(); }

    private:
        ListBenTypes benlist_;
    };

    class BenDictionary : public BenType, private NotCopyable
    {
        typedef std::map<std::string, std::tr1::shared_ptr<BenType> > BenMap;
    public:
        typedef std::string key_type;
        typedef BenMap::mapped_type value_type;
        typedef BenMap::value_type pair_type;
        typedef BenMap::iterator iterator;
        typedef BenMap::const_iterator const_iterator;

        explicit BenDictionary(BenTypesStreamBuf& buf);

        std::size_t size() const { return benmap_.size(); }

        iterator begin() { return benmap_.begin(); }
        const_iterator begin() const { return benmap_.begin(); }

        iterator end() { return benmap_.end(); }
        const_iterator end() const { return benmap_.end(); }

        const_iterator find(const std::string& key) const { return benmap_.find(key); }

        // return a empty value_type object if can not find key
        value_type operator [] (const std::string& key) const
        {
            const_iterator it = find(key);
            if (it == benmap_.end()) return value_type();
            return it->second;
        }

    private:
        BenMap benmap_;
    };

    // return a empty shared_ptr if buf is EOF
    std::tr1::shared_ptr<BenType> GetBenObject(BenTypesStreamBuf& buf);

} // namespace bentypes

#endif // _BEN_TYPES_H_

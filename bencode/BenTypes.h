#ifndef _BEN_TYPES_H_
#define _BEN_TYPES_H_

#include <string>
#include <list>
#include <map>
#include <vector>
#include <stdio.h>

#include "../base/BaseTypes.h"

namespace bentypes
{
    class BenTypesStreamBuf : private NotCopyable
    {
    public:
        BenTypesStreamBuf(const char *buf, std::size_t size);
        BenTypesStreamBuf(FILE *pfile, std::size_t size);

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

        std::size_t Length() const { return benstr_.size(); }
        const char * c_str() const { return benstr_.c_str(); }
        std::string std_string() const { return benstr_; }

    private:
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
        typedef std::list<BenType *> ListBenTypes;
    public:
        typedef ListBenTypes::iterator iterator;
        typedef ListBenTypes::const_iterator const_iterator;
        typedef BenType * value_type;

        explicit BenList(BenTypesStreamBuf& buf);
        virtual ~BenList();

        std::size_t Count() const { return benlist_.size(); }

        iterator begin() { return benlist_.begin(); }
        const_iterator begin() const { return benlist_.begin(); }

        iterator end() { return benlist_.end(); }
        const_iterator end() const { return benlist_.end(); }

    private:
        std::list<BenType *> benlist_;
    };

    class BenDictionary : public BenType, private NotCopyable
    {
        typedef std::map<std::string, BenType *> BenMap;
    public:
        typedef std::string key_type;
        typedef BenType * value_type;
        typedef BenMap::value_type pair_type;
        typedef BenMap::iterator iterator;
        typedef BenMap::const_iterator const_iterator;

        explicit BenDictionary(BenTypesStreamBuf& buf);
        virtual ~BenDictionary();

        std::size_t size() const { return benmap_.size(); }

        iterator begin() { return benmap_.begin(); }
        const_iterator begin() const { return benmap_.begin(); }

        iterator end() { return benmap_.end(); }
        const_iterator end() const { return benmap_.end(); }

        const_iterator find(const std::string& key) const { return benmap_.find(key); }
        value_type operator [] (const std::string& key) const
        {
            const_iterator it = find(key);
            if (it == benmap_.end()) return 0;
            return it->second;
        }

    private:
        BenMap benmap_;
    };

} // namespace bentypes

#endif // _BEN_TYPES_H_

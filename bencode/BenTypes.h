#ifndef _BEN_TYPES_H_
#define _BEN_TYPES_H_

#include <string>
#include <list>
#include <map>

#include "../base/BaseTypes.h"

namespace bentypes
{
    class BenTypesStreamBuf
    {
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
        std::size_t Length() const;
        operator const char * ();

    private:
        std::string benstr_;
    };

    class BenInteger : public BenType
    {
    public:
        explicit BenInteger(BenTypesStreamBuf& buf);
        operator int ();

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
        std::size_t Count() const;

        iterator begin();
        const_iterator begin() const;

        iterator end();
        const_iterator end() const;

    private:
        std::list<BenType *> benlist_;
    };

    class BenDictionary : public BenType, private NotCopyable
    {
        typedef std::map<BenString, BenType *> BenMap;
    public:
        typedef BenString key_type;
        typedef BenType * value_type;
        typedef BenMap::value_type pair_type;

        explicit BenDictionary(BenTypesStreamBuf& buf);
        virtual ~BenDictionary();

    private:
        BenMap benmap_;
    };

} // namespace bentypes

#endif // _BEN_TYPES_H_
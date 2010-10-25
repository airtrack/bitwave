#include "../unittest/UnitTest.h"
#include "../bencode/BenTypes.h"
#include <string.h>

using namespace bittorrent;
using namespace bittorrent::bentypes;

TEST_CASE(BenInteger)
{
    const char *beninteger = "i3e";
    BenTypesStreamBuf buf(beninteger, strlen(beninteger));
    BenTypesStreamBuf::const_iterator begin = buf.begin();
    BenTypesStreamBuf::const_iterator end = buf.end();
    BenInteger i(begin, end);
    CHECK_TRUE(i.GetValue() == 3);
}

TEST_CASE(BenString)
{
    const char *benstring = "4:spam";
    BenTypesStreamBuf buf(benstring, strlen(benstring));
    BenTypesStreamBuf::const_iterator begin = buf.begin();
    BenTypesStreamBuf::const_iterator end = buf.end();
    BenString s(begin, end);
    CHECK_TRUE(s.std_string() == "spam");
}

TEST_CASE(BenList)
{
    const char *benlist = "l4:spam4:eggse";
    BenTypesStreamBuf buf(benlist, strlen(benlist));
    BenTypesStreamBuf::const_iterator begin = buf.begin();
    BenTypesStreamBuf::const_iterator end = buf.end();
    BenList l(begin, end);
    CHECK_TRUE(l.size() == 2);

    std::vector<BenString *> benstrings;
    l.AllElementPtr(&benstrings);
    CHECK_TRUE(benstrings.size() == 2);
    CHECK_TRUE(benstrings[0]->std_string() == "spam");
    CHECK_TRUE(benstrings[1]->std_string() == "eggs");
}

TEST_CASE(BenDictionary)
{
    const char *bendict = "d3:cow3:moo4:spam4:eggse";
    BenTypesStreamBuf buf(bendict, strlen(bendict));
    BenTypesStreamBuf::const_iterator begin = buf.begin();
    BenTypesStreamBuf::const_iterator end = buf.end();
    BenDictionary d(begin, end);
    CHECK_TRUE(d.size() == 2);
    BenDictionary::value_type v1 = d["cow"];
    BenDictionary::value_type v2 = d["spam"];
    BenString *pv1 = dynamic_cast<BenString *>(v1.get());
    BenString *pv2 = dynamic_cast<BenString *>(v2.get());
    CHECK_TRUE(v1 && pv1);
    CHECK_TRUE(v2 && pv2);
    CHECK_TRUE(pv1->std_string() == "moo");
    CHECK_TRUE(pv2->std_string() == "eggs");
}

int main()
{
    TestCollector.RunCases();

    return 0;
}

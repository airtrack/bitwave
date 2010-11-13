#include "../protocol/URI.h"
#include "../unittest/UnitTest.h"

using namespace bittorrent;

TEST_CASE(host_name)
{
    URI uri("http://www.sample.com");
    std::string host = uri.GetHost();
    CHECK_TRUE(host == "www.sample.com");
}

TEST_CASE(querys)
{
    URI uri("http://www.sample.com");
    uri.AddQuery("id", 123);
    uri.AddQuery("data", "456", 3);
    std::string uristr = uri.GetURIString();
    CHECK_TRUE(uristr == "http://www.sample.com?id=123&data=456");
}

int main()
{
    TestCollector.RunCases();
    return 0;
}

#include "../protocol/URI.h"
#include "../unittest/UnitTest.h"

using namespace bittorrent;
using namespace http;

TEST_CASE(host_name)
{
    URI uri("http://www.sample.com");
    std::string host = uri.GetHost();
    CHECK_TRUE(host == "www.sample.com");
}

TEST_CASE(querys)
{
    char data[2] = { 0x0F, 0x0E };
    URI uri("http://www.sample.com/announce.php");
    uri.AddQuery("id", 123);
    uri.AddQuery("data", data, 2);
    std::string uristr = uri.GetQueryString();
    CHECK_TRUE(uristr == "/announce.php?id=123&data=%0F%0E");
}

int main()
{
    TestCollector.RunCases();
    return 0;
}

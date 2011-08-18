#include "../log/LogFactory.h"
#include "../log/Manipulator.h"
#include <string>

using namespace bitwave;
using namespace log;

int main()
{
    CompositeLogFactory<> log_factory;
    CompositeLogFactory<>::LogImplPtr log_impl = log_factory
        .MakeCoutLog().StoreLog()
        .MakeNullLog().StoreLog()
        .MakeCoutLog().StoreLog()
        .FetchLog().LogLeaveFactory();
    Log<> log(log_impl);

    log << "abcdefg" << Endl;
    log << 123456 << Endl;
    log << 1.23456 << Endl;
    log << std::string("hijklmn") << Endl;

    log << RecordStarter << "this is record start" << Endl;
    log << "this is record" << Endl;
    log << "this is record end\n" << RecordEnder;

    return 0;
}

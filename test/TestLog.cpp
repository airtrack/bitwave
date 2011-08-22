#include "../log/LogFactory.h"
#include "../log/Manipulator.h"
#include <string>

using namespace bitwave;
using namespace log;

int main()
{
    CompositeLogFactory<> log_factory;
    CompositeLogFactory<>::LogImplPtr log_impl = log_factory
        .MakeTextFileLog("log.txt").AddTimestamp().StoreLog()
        .MakeCoutLog().AddTimestamp().StoreLog()
        .MakeNullLog().StoreLog()
        .FetchLog().LogLeaveFactory();
    Log<> log(log_impl);

    log << "abcdefg" << Endl;
    log << 123456 << Endl;
    log << 1.23456 << Endl;
    log << RecordStarter << std::string("hijklmn") << Endl << RecordEnder;

    log << RecordStarter << "this is record start" << Endl;
    log << "this is record" << Endl;
    log << "this is record end\n" << RecordEnder;

    return 0;
}

#include "../log/LogFactory.h"
#include "../log/Manipulator.h"
#include <string>

using namespace bitwave;
using namespace log;

int main()
{
    LogFactory<> log_factory;
    LogFactory<>::LogImplPtr log_impl =
        log_factory.MakeCoutLog().LogLeaveFactory();
    Log<> log(log_impl);

    log << "abcdefg" << Endl;
    log << 123456 << Endl;
    log << 1.23456 << Endl;
    log << std::string("hijklmn") << Endl;

    log << RecordStarter << "this is record start" << Endl;
    log << "this is record" << Endl;
    log << "this is record end" << RecordEnder;

    return 0;
}

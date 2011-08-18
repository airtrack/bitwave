#ifndef MANIPULATOR_H
#define MANIPULATOR_H

namespace bitwave {
namespace log {

    template<typename LogType>
    inline void Endl(LogType& log)
    {
        log << '\n';
        log.Flush();
    }

    template<typename LogType>
    inline void RecordStarter(LogType& log)
    {
        log.StartRecord();
    }

    template<typename LogType>
    inline void RecordEnder(LogType& log)
    {
        log.EndRecord();
    }

} // namespace log
} // namespace bitwave

#endif // MANIPULATOR_H

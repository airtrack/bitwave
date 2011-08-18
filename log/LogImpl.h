#ifndef LOG_IMPL_H
#define LOG_IMPL_H

#include <string>
#include <iostream>

namespace bitwave {
namespace log {

    template<typename CharType>
    class LogImplBase
    {
    public:
        virtual void StartLogRecord() = 0;
        virtual void LogRecord(const std::basic_string<CharType>& record) = 0;
        virtual void EndLogRecord() = 0;
        virtual ~LogImplBase() { }
    };

    template<typename CharType>
    class NullLogImpl : public LogImplBase<CharType>
    {
    public:
        virtual void StartLogRecord() { }
        virtual void LogRecord(const std::basic_string<CharType>&) { }
        virtual void EndLogRecord() { }
    };

    template<typename CharType>
    class CoutLogImpl;

    template<>
    class CoutLogImpl<char> : public LogImplBase<char>
    {
    public:
        virtual void StartLogRecord() { }
        virtual void LogRecord(const std::string& log) { std::cout << log; }
        virtual void EndLogRecord() { }
    };

    template<>
    class CoutLogImpl<wchar_t> : public LogImplBase<wchar_t>
    {
    public:
        virtual void StartLogRecord() { }
        virtual void LogRecord(const std::wstring& log) { std::wcout << log; }
        virtual void EndLogRecord() { }
    };

} // namespace log
} // namespace bitwave

#endif // LOG_IMPL_H

#ifndef LOG_IMPL_H
#define LOG_IMPL_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

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

    template<typename CharType>
    class CompositeLogImpl : public LogImplBase<CharType>
    {
    public:
        typedef LogImplBase<CharType> BaseType;
        typedef std::tr1::shared_ptr<BaseType> BaseTypePtr;

        void AddLogImpl(const BaseTypePtr& log_impl)
        {
            logs_.push_back(log_impl);
        }

        virtual void StartLogRecord()
        {
            std::for_each(logs_.begin(), logs_.end(),
                    std::tr1::bind(&BaseType::StartLogRecord,
                        std::tr1::placeholders::_1));
        }

        virtual void LogRecord(const std::basic_string<CharType>& log)
        {
            std::for_each(logs_.begin(), logs_.end(),
                    std::tr1::bind(&BaseType::LogRecord,
                        std::tr1::placeholders::_1, std::tr1::cref(log)));
        }

        virtual void EndLogRecord()
        {
            std::for_each(logs_.begin(), logs_.end(),
                    std::tr1::bind(&BaseType::EndLogRecord,
                        std::tr1::placeholders::_1));
        }

    private:
        std::vector<BaseTypePtr> logs_;
    };

} // namespace log
} // namespace bitwave

#endif // LOG_IMPL_H

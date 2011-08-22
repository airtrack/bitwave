#ifndef LOG_IMPL_H
#define LOG_IMPL_H

#include <time.h>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
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

    template<typename CharType>
    class TextFileLogImpl : public LogImplBase<CharType>
    {
    public:
        TextFileLogImpl();

        virtual void StartLogRecord() { }
        virtual void LogRecord(const std::basic_string<CharType>& log)
        {
        }
        virtual void EndLogRecord() { }

    private:
    };

    template<typename CharType>
    class TimestampAdderLogImpl : public LogImplBase<CharType>
    {
    public:
        typedef LogImplBase<CharType> BaseType;
        typedef std::tr1::shared_ptr<BaseType> BaseTypePtr;

        explicit TimestampAdderLogImpl(const BaseTypePtr& log)
            : log_(log)
        {
        }

        virtual void StartLogRecord()
        {
            // add timestamp
            time_t now = time(0);
            tm *ptm = localtime(&now);
            ConvertTimeToString(ptm, CharType());
            log_->LogRecord(timestamp_.str());
            timestamp_.str(std::basic_string<CharType>());

            log_->StartLogRecord();
        }

        virtual void LogRecord(const std::basic_string<CharType>& log)
        {
            log_->LogRecord(log);
        }

        virtual void EndLogRecord()
        {
            log_->EndLogRecord();
        }

    private:
        void ConvertTimeToString(const tm *ptm, char)
        {
            timestamp_ << '[' << 1900 + ptm->tm_year << '-'
                << 1 + ptm->tm_mon << '-' << ptm->tm_mday << ' '
                << ptm->tm_hour<< ':' << ptm->tm_min << ':'
                << ptm->tm_sec << ']';
        }

        void ConvertTimeToString(const tm *ptm, wchar_t)
        {
            timestamp_ << L'[' << 1900 + ptm->tm_year << L'-'
                << 1 + ptm->tm_mon << L'-' << ptm->tm_mday << L' '
                << ptm->tm_hour<< L':' << ptm->tm_min << L':'
                << ptm->tm_sec << L']';
        }

        std::basic_stringstream<CharType> timestamp_;
        BaseTypePtr log_;
    };

} // namespace log
} // namespace bitwave

#endif // LOG_IMPL_H

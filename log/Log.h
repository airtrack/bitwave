#ifndef LOG_H
#define LOG_H

#include "LogImpl.h"
#include "../base/BaseTypes.h"
#include <memory>
#include <sstream>

namespace bitwave {
namespace log {

    template<typename CharType = char>
    class Log : private NotCopyable
    {
    public:
        // function prototype of manipulator for the log
        typedef void (*Manipulator)(Log& log);
        typedef LogImplBase<CharType> LogImpl;
        typedef std::tr1::shared_ptr<LogImpl> LogImplPtr;

        explicit Log(const LogImplPtr& log_impl)
            : new_record_(false),
              record_stream_(),
              log_impl_ptr_(log_impl)
        {
        }

        // a generic operator << to record any type T to the record
        // stream buffer
        template<typename T>
        Log& operator << (const T& t)
        {
            record_stream_ << t;
            return *this;
        }

        // operator << for Manipulator to do some special operation
        // on the log
        Log& operator << (Manipulator manipulator)
        {
            manipulator(*this);
            return *this;
        }

        // write record stream buffer to implement log and clear the
        // stream buffer.
        void Flush()
        {
            log_impl_ptr_->LogRecord(record_stream_.str());
            record_stream_.str("");
        }

        // start a record, all operator << call after this function
        // will in the record before end the record
        // this is an optional operator, call this function you can
        // start a record explicitly, then after log some information
        // you need call EndRecord function to end the record explicitly
        void StartRecord()
        {
            if (!new_record_)
            {
                log_impl_ptr_->StartLogRecord();
                new_record_ = true;
            }
        }

        // end a record, all operator << call before this function
        // will in the record after start the record
        // call this function you must called StartRecord, this function
        // end the record explicitly
        void EndRecord()
        {
            if (new_record_)
            {
                Flush();
                log_impl_ptr_->EndLogRecord();
                new_record_ = false;
            }
        }

    private:
        // record mode flag
        bool new_record_;
        // record stream buffer
        std::basic_stringstream<CharType> record_stream_;
        // implement log
        LogImplPtr log_impl_ptr_;
    };

} // namespace log
} // namespace bitwave

#endif // LOG_H

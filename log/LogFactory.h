#ifndef LOG_FACTORY_H
#define LOG_FACTORY_H

#include "Log.h"
#include "../base/BaseTypes.h"

namespace bitwave {
namespace log {

    template<typename CharType = char>
    class LogFactory : private NotCopyable
    {
    public:
        typedef typename Log<CharType>::LogImplPtr LogImplPtr;

        LogFactory& MakeNullLog()
        {
            log_impl_ptr_.reset(new NullLogImpl<CharType>);
            return *this;
        }

        LogFactory& MakeCoutLog()
        {
            log_impl_ptr_.reset(new CoutLogImpl<CharType>);
            return *this;
        }

        LogImplPtr LogLeaveFactory()
        {
            LogImplPtr product = log_impl_ptr_;
            log_impl_ptr_.reset();
            return product;
        }

    private:
        LogImplPtr log_impl_ptr_;
    };

} // namespace log
} // namespace bitwave

#endif // LOG_FACTORY_H

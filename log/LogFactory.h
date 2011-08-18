#ifndef LOG_FACTORY_H
#define LOG_FACTORY_H

#include "Log.h"
#include "../base/BaseTypes.h"

namespace bitwave {
namespace log {

    // this is log implement factory, it can produce a log product through
    // several process operations.
    template<typename CharType = char>
    class LogFactory : private NotCopyable
    {
    public:
        typedef typename Log<CharType>::LogImplPtr LogImplPtr;

        // make a null log implement as current product
        LogFactory& MakeNullLog()
        {
            product_ptr_.reset(new NullLogImpl<CharType>);
            return *this;
        }

        // make a cout log implement as current product
        LogFactory& MakeCoutLog()
        {
            product_ptr_.reset(new CoutLogImpl<CharType>);
            return *this;
        }

        // this function do nothing. derived factory could
        // override this function to store current product
        virtual LogFactory& StoreLog()
        {
            return *this;
        }

        // this function do nothing. derived factory could
        // override this function to fetch stored product
        virtual LogFactory& FetchLog()
        {
            return *this;
        }

        // get the log product from the factory
        LogImplPtr LogLeaveFactory()
        {
            LogImplPtr product = product_ptr_;
            product_ptr_.reset();
            return product;
        }

    protected:
        LogImplPtr product_ptr_;
    };

    // this factory produce CompositeLogImpl by composite several products
    // which produced by base LogFactory.
    template<typename CharType = char>
    class CompositeLogFactory : public LogFactory<CharType>
    {
    public:
        CompositeLogFactory()
            : composite_log_(new CompositeLogImpl<CharType>)
        {
        }

        virtual CompositeLogFactory& StoreLog()
        {
            if (product_ptr_)
            {
                // composite current product
                composite_log_->AddLogImpl(product_ptr_);
                product_ptr_.reset();
            }
            return *this;
        }

        virtual CompositeLogFactory& FetchLog()
        {
            // make the composite_log_ as product_ptr_
            // then LogLeaveFactory will get the composite_log_
            product_ptr_ = composite_log_;
            composite_log_.reset(new CompositeLogImpl<CharType>);
            return *this;
        }

    private:
        std::tr1::shared_ptr<CompositeLogImpl<CharType>> composite_log_;
    };

} // namespace log
} // namespace bitwave

#endif // LOG_FACTORY_H

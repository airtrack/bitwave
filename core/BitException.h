#ifndef BIT_EXCEPTION_H
#define BIT_EXCEPTION_H

#include "../base/BaseTypes.h"

namespace bitwave {
namespace core {

    // bentypes exception enum code
    enum BenTypeExceptionCode
    {
        INVALIDATE_NOBENTYPE,
        INVALIDATE_INTERGER,
        INVALIDATE_STRING,
        INVALIDATE_LIST,
        INVALIDATE_DICTIONARY,
    };

    // bentypes exception
    class BenTypeException : public BaseException
    {
    public:
        explicit BenTypeException(int code)
            : BaseException(code)
        {
        }
    };

    class MetainfoFileExeception : public BaseException
    {
    public:
        explicit MetainfoFileExeception(const std::string& w)
            : BaseException(w)
        {
        }
    };

    enum FileExceptionCode
    {
        PATH_ERROR,
        SPACE_NOT_ENOUGH,
    };

    class CreateFileException : public BaseException
    {
    public:
        CreateFileException(int code, const std::string& file_path)
            : BaseException(code),
              file_path_(file_path)
        {
        }

        std::string GetFilePath() const
        {
            return file_path_;
        }

    private:
        std::string file_path_;
    };

    // an exception class for BitPeerListener
    class ListenPortException : public BaseException
    {
    public:
        explicit ListenPortException(const std::string& w)
            : BaseException(w)
        {
        }
    };

} // namespace core
} // namespace bitwave

#endif // BIT_EXCEPTION_H

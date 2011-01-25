#include "Response.h"
#include <assert.h>
#include <stdlib.h>
#include <string>

namespace bittorrent {
namespace http {

    // an exception class of construct StatusLine
    class InvalidateStatusLine {};

    class StatusLine
    {
    public:
        StatusLine(const char *begin, const char *end)
            : status_line_data_len_(0), status_code_(0),
              http_version_(0), reason_phrase_()
        {
            ParseData(begin, end);
        }

        int GetStatusLineLength() const
        {
            return status_line_data_len_;
        }

        int GetStatusCode() const
        {
            return status_code_;
        }

        int GetStatusCodeType() const
        {
            return status_code_ / 100;
        }

        double GetHttpVersion() const
        {
            return http_version_;
        }

        std::string GetReasonPhrase() const
        {
            return reason_phrase_;
        }

    private:
        void ParseData(const char *begin, const char *end)
        {
            assert(begin && end);
            const char *pp = begin;
            const char *ptr = begin;
            PointerTo(ptr, end, ' ');

            std::string http_version(pp, ptr);
            ParseHttpVersion(http_version);

            pp = ++ptr;
            PointerTo(ptr, end, ' ');

            std::string status_code(pp, ptr);
            ParseStatusCode(status_code);

            pp = ++ptr;
            PointerTo(ptr, end, '\r');

            std::string reason_phrase(pp, ptr);
            ParseReasonPhrase(reason_phrase);

            if (++ptr == end || *ptr++ != '\n')
                throw InvalidateStatusLine();

            status_line_data_len_ = ptr - begin;
        }

        void PointerTo(const char *&ptr, const char *end, char c)
        {
            for (; ptr != end && *ptr != c; ++ptr);
            if (ptr == end)
                throw InvalidateStatusLine();
        }

        void ParseHttpVersion(const std::string& version)
        {
            std::string::size_type i = version.find("HTTP/");
            if (i == std::string::npos)
                throw InvalidateStatusLine();

            http_version_ = atof(version.substr(i + 5).c_str());
        }

        void ParseStatusCode(const std::string& code)
        {
            status_code_ = atoi(code.c_str());
        }

        void ParseReasonPhrase(const std::string& reason)
        {
            reason_phrase_ = reason;
        }

        int status_line_data_len_;
        int status_code_;
        double http_version_;
        std::string reason_phrase_;
    };

    // an exception class of ContentLocator
    class CanNotLocateContent {};

    class ContentLocator
    {
    public:
        ContentLocator(const char *stream, std::size_t size)
            : content_begin_pos_(0),
              content_length_(0),
              response_length_(0)
        {
            ParseContent(stream, size);
        }

        const char * GetContentBeginPos() const
        {
            return content_begin_pos_;
        }

        std::size_t GetContentLength() const
        {
            return content_length_;
        }

        std::size_t GetResponseLength() const
        {
            return response_length_;
        }

    private:
        void ParseContent(const char *stream, std::size_t size)
        {
            assert(stream && size > 0);
            StatusLine status_line(stream, stream + size);
            // FIXME: use regex
            std::size_t status_line_len = status_line.GetStatusLineLength();
            if (size - status_line_len <= 0)
                throw CanNotLocateContent();

            // from StatusLine's \r\n to end
            std::string str(stream + status_line_len - 2, size - status_line_len + 2);
            std::string::size_type end_pos = str.find("\r\n\r\n");
            if (end_pos == std::string::npos)
                throw CanNotLocateContent();

            std::string::size_type pos = str.find("Content-Length");
            if (pos != std::string::npos)
            {
                std::string::size_type len_begin = str.find(':', pos) + 1;
                std::string::size_type len_end = str.find("\r\n", len_begin);
                content_length_ = atoi(str.substr(len_begin, len_end).c_str());
            }

            response_length_ = status_line_len + end_pos + content_length_ + 2;
            if (response_length_ > size)
                throw CanNotLocateContent();

            content_begin_pos_ = stream + response_length_ - content_length_;
        }

        const char *content_begin_pos_;
        std::size_t content_length_;
        std::size_t response_length_;
    };

    // static
    bool ResponseUnpackRuler::CanUnpack(const char *stream,
            std::size_t size, std::size_t *pack_len)
    {
        assert(stream && pack_len);
        try
        {
            ContentLocator checker(stream, size);
            *pack_len = checker.GetResponseLength();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    Response::Response(const char *stream, std::size_t size)
        : status_code_(0),
          status_code_type_(0),
          http_version_(0),
          reason_phrase_(),
          content_buffer_()
    {
        try
        {
            assert(stream);
            StatusLine status_line(stream, stream + size);
            status_code_ = status_line.GetStatusCode();
            status_code_type_ = status_line.GetStatusCodeType();
            http_version_ = status_line.GetHttpVersion();
            reason_phrase_ = status_line.GetReasonPhrase();

            ContentLocator locator(stream, size);
            const char *content_begin_pos = locator.GetContentBeginPos();
            std::size_t content_length = locator.GetContentLength();
            content_buffer_.assign(content_begin_pos, content_begin_pos + content_length);
        }
        catch (...)
        {
            throw InvalidateResponse();
        }
    }

    int Response::GetStatusCode() const
    {
        return status_code_;
    }

    int Response::GetStatusCodeType() const
    {
        return status_code_type_;
    }

    double Response::GetHttpVersion() const
    {
        return http_version_;
    }

    std::string Response::GetReasonPhrase() const
    {
        return reason_phrase_;
    }

    const char * Response::GetContentBufPointer() const
    {
        if (GetContentSize() == 0)
            return 0;
        return &content_buffer_[0];
    }

    std::size_t Response::GetContentSize() const
    {
        return content_buffer_.size();
    }

} // namespace http
} // namespace bittorrent

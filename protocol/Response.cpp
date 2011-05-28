#include "Response.h"
#include "HttpException.h"
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <string>

namespace {

    // an exception class of construct StatusLine
    class InvalidateStatusLine {};

    class StatusLine
    {
    public:
        StatusLine(const char *begin, const char *end)
            : status_line_data_len_(0),
              status_code_(0),
              http_version_(0),
              reason_phrase_()
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

            ParseHttpVersion(pp, ptr);

            pp = ++ptr;
            PointerTo(ptr, end, ' ');

            ParseStatusCode(pp, ptr);

            pp = ++ptr;
            PointerTo(ptr, end, '\r');

            ParseReasonPhrase(pp, ptr);

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

        void ParseHttpVersion(const char *begin, const char *end)
        {
            const char http[] = "HTTP/";
            const char *result = std::search(begin, end, http, http + sizeof(http) - 1);
            if (result == end)
                throw InvalidateStatusLine();

            http_version_ = atof(std::string(result + sizeof(http) - 1, end).c_str());
        }

        void ParseStatusCode(const char *begin, const char *end)
        {
            status_code_ = atoi(std::string(begin, end).c_str());
        }

        void ParseReasonPhrase(const char *begin, const char *end)
        {
            reason_phrase_.assign(begin, end);
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
            std::size_t status_line_len = status_line.GetStatusLineLength();
            if (size - status_line_len <= 0)
                throw CanNotLocateContent();

            const char CRLF[] = "\r\n";
            const char d_CRLF[] = "\r\n\r\n";
            const char content_len[] = "Content-Length";

            // from StatusLine's \r\n to end
            const char *begin = stream + status_line_len - (sizeof(CRLF) - 1);
            const char *end = stream + size;

            const char *head_end_pos = std::search(begin, end, d_CRLF, d_CRLF + sizeof(d_CRLF) - 1);
            if (head_end_pos == end)
                throw CanNotLocateContent();

            const char *content_len_pos = std::search(begin, end, content_len, content_len + sizeof(content_len) - 1);
            if (content_len_pos != end)
            {
                const char *len_begin = std::find(content_len_pos, end, ':') + 1;
                const char *len_end = std::search(len_begin, end, CRLF, CRLF + sizeof(CRLF) - 1);
                content_length_ = atoi(std::string(len_begin, len_end).c_str());
            }

            response_length_ = status_line_len + (head_end_pos - begin) + content_length_ + sizeof(d_CRLF) - sizeof(CRLF);
            if (response_length_ > size)
                throw CanNotLocateContent();

            content_begin_pos_ = stream + response_length_ - content_length_;
        }

        const char *content_begin_pos_;
        std::size_t content_length_;
        std::size_t response_length_;
    };

} // unnamed namespace

namespace bitwave {
namespace http {

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
            throw ResponseException(std::string(stream, size));
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
} // namespace bitwave

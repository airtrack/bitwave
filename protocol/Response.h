#ifndef RESPONSE_H
#define RESPONSE_H

#include <vector>

namespace bitwave {
namespace http {

    // a simply http Response unpack ruler to unpack tcp stream
    class ResponseUnpackRuler
    {
    public:
        static bool CanUnpack(const char *stream,
                std::size_t size, std::size_t *pack_len);
    };

    // an exception class of construct Response
    class InvalidateResponse {};

    class Response
    {
    public:
        enum {
            INFORMATION = 1,
            SUCCESS,
            REDIRECTION,
            CLIENT_ERROR,
            SERVER_ERROR
        };

        Response(const char *stream, std::size_t size);

        int GetStatusCode() const;
        int GetStatusCodeType() const;
        double GetHttpVersion() const;
        std::string GetReasonPhrase() const;

        // get response content, pointer to content buffer,
        // return zero when the response has no content
        const char * GetContentBufPointer() const;
        std::size_t GetContentSize() const;

    private:
        typedef std::vector<char> ContentBuffer;

        int status_code_;
        int status_code_type_;
        double http_version_;
        std::string reason_phrase_;
        ContentBuffer content_buffer_;
    };

} // namespace http
} // namespace bitwave

#endif // RESPONSE_H

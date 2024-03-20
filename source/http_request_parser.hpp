#ifndef HTTP_REQUEST_PARSER_hpp
#define HTTP_REQUEST_PARSER_hpp

#include "config.hpp"
#include "http_request.hpp"

#include <stdexcept>

#define HTTP_REQUEST_HEADER_MAX_LENGTH 8192

enum HttpRequestPhase
{
    // In-progress phases
    HTTP_REQUEST_HEADER,
    HTTP_REQUEST_BODY_RAW,
    HTTP_REQUEST_BODY_CHUNKED_HEADER,
    HTTP_REQUEST_BODY_CHUNKED_BODY,
    HTTP_REQUEST_BODY_CHUNKED_CRLF,

    // Final phases
    HTTP_REQUEST_HEADER_EXCEED,
    HTTP_REQUEST_BODY_EXCEED,
    HTTP_REQUEST_MALFORMED,
    HTTP_REQUEST_COMPLETED
};

class HttpRequestParser
{
public:
    /* Constructs a HTTP request parser using the given rules */
    HttpRequestParser(const ServerConfig &config, uint32_t host, uint16_t port);

    /* Prepares the parser to consume the next request */
    void reset();

    /* Commits data to the parser, returns whether the parser has transitioned into a final phase */
    bool commit(Slice &data);

    /* Gets the parser's current phase */
    inline HttpRequestPhase getPhase() const
    {
        return _phase;
    }

    /* Gets the built request; only valid when the current phase is `HTTP_REQUEST_COMPLETED` */
    inline HttpRequest getRequest() const
    {
        if (_phase != HTTP_REQUEST_COMPLETED)
            throw std::runtime_error("Attempt to access incomplete or malformed request");
        return _request;
    }
private:
    const ServerConfig &_config;
    uint32_t            _host;
    uint16_t            _port;
    HttpRequest         _request;
    HttpRequestPhase    _phase;
    char                _headerBuffer[HTTP_REQUEST_HEADER_MAX_LENGTH];
    size_t              _headerLength;
    size_t              _bodyRemainder;

    /* Handles a data commit in the `HTTP_REQUEST_HEADER` phase */
    HttpRequestPhase handleHeader(Slice &data);

    /* Handles a data commit in the `HTTP_REQUEST_BODY_RAW` phase */
    HttpRequestPhase handleBodyRaw(Slice &data);

    /* Handles a data commit in the `HTTP_REQUEST_BODY_CHUNKED_HEADER` phase */
    HttpRequestPhase handleBodyChunkedHeader(Slice &data);

    /* Handles a data commit in the `HTTP_REQUEST_BODY_CHUNKED_BODY` phase */
    HttpRequestPhase handleBodyChunkedBody(Slice &data);

    /* Handles a data commit in the `HTTP_REQUEST_BODY_CHUNKED_CRLF` phase */
    HttpRequestPhase handleBodyChunkedCRLF(Slice &data);

    /* Parses the given HTTP header */
    bool parseHeader(Slice data);
};

#endif // HTTP_REQUEST_PARSER_hpp

#ifndef HTTP_REQUEST_PARSER_hpp
#define HTTP_REQUEST_PARSER_hpp

#include "config.hpp"
#include "http_request.hpp"

class HttpRequestParser
{
public:
    /* Constructs a HTTP request parser using the given rules */
    HttpRequestParser(const ServerConfig &config, uint32_t host, uint16_t port);

    /* Commits the given buffer to the current request
        Returns true if parsing was finished and moves the parsed request into `outRequest` */
    bool commit(const char *buffer, size_t length, HttpRequest &outRequest);
};

#endif // HTTP_REQUEST_PARSER_hpp

#include "http_request.hpp"

/* Constructs a HTTP request parser using the given rules */
HttpRequest::Parser::Parser(const ServerConfig &config, uint32_t host, uint16_t port)
{
    (void)config;
    (void)host;
    (void)port;
}

/* Commits the given buffer to the current request
    Returns true if parsing was finished and moves the parsed request into `outRequest` */
bool HttpRequest::Parser::commit(const char *buffer, size_t length, HttpRequest &outRequest)
{
    (void)buffer;
    (void)length;
    (void)outRequest;
    return false;
}

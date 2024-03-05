#include "http_request.hpp"

/* Constructs a HTTP header pair using its key and value */
HttpRequest::Header::Header(const std::string &key, const std::string &value)
    : _key(key)
    , _value(value)
{
}

/* Case-invariantly checks if the given key matches with the header's key */
bool HttpRequest::Header::matchKey(Slice key) const
{
    if (_key.size() != key.getLength())
        return false;
    for (size_t index = 0; index < key.getLength(); index++)
    {
        if (std::tolower(_key[index]) != std::tolower(key[index]))
            return false;
    }
    return true;
}

/* Case-invariantly finds a header in the given vector of headers */
const HttpRequest::Header *HttpRequest::findHeaderIn(const std::vector<Header> &headers, Slice key)
{
    for (size_t index = 0; index < headers.size(); index++)
    {
        if (headers[index].matchKey(key))
            return &headers[index];
    }
    return NULL;
}

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

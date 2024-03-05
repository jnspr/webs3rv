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

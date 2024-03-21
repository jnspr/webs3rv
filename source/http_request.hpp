#ifndef HTTP_REQUEST_hpp
#define HTTP_REQUEST_hpp

#include "slice.hpp"
#include "http_constants.hpp"

#include <string>
#include <vector>
#include <stddef.h>
#include <stdint.h>

struct HttpRequest
{
    class Header
    {
    public:
        /* Constructs a HTTP header pair using its key and value */
        Header(const std::string &key, const std::string &value);

        /* Case-invariantly checks if the given key matches with the header's key */
        bool matchKey(Slice key) const;

        /* Gets the header's key */
        inline const std::string &getKey() const
        {
            return _key;
        }

        /* Gets the header's value */
        inline const std::string &getValue() const
        {
            return _value;
        }
    private:
        std::string _key;
        std::string _value;
    };

    HttpMethod           method;
    uint32_t             clientHost;
    uint16_t             clientPort;
    std::string          query;           // Full URL-encoded query string; eg. "/cgi-bin/demo.py?hello=world&abc=def"
    std::string          queryPath;       // URL-decoded query path; eg. "/cgi-bin/demo.py"
    Slice                queryParameters; // URL-encoded Query parameters; eg. "hello=world&abc=def"
    bool                 isLegacy;        // True when HTTP/1.0 instead of HTTP/1.1
    std::vector<Header>  headers;
    std::vector<uint8_t> body;

    /* Case-invariantly finds a header in the current request */
    inline const Header *findHeader(Slice key)
    {
        return findHeaderIn(headers, key);
    }

    /* Case-invariantly finds a header in the given vector of headers */
    static const Header *findHeaderIn(const std::vector<Header> &headers, Slice key);
};

#endif // HTTP_REQUEST_hpp

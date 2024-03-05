#ifndef HTTP_REQUEST_hpp
#define HTTP_REQUEST_hpp

#include "slice.hpp"
#include "config.hpp"
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
        inline const std::string &getKey()
        {
            return _key;
        }

        /* Gets the header's value */
        inline const std::string &getValue()
        {
            return _value;
        }
    private:
        std::string _key;
        std::string _value;
    };

    class Parser
    {
    public:
        /* Constructs a HTTP request parser using the given rules */
        Parser(const ServerConfig &config, uint32_t host, uint16_t port);

        /* Commits the given buffer to the current request
           Returns true if parsing was finished and moves the parsed request into `outRequest` */
        bool commit(const char *buffer, size_t length, HttpRequest &outRequest);
    };

    HttpMethod           method;
    uint32_t             clientHost;
    uint16_t             clientPort;
    std::string          query;           // Full query string; eg. "/cgi-bin/demo.py?hello=world&abc=def"
    Slice                queryPath;       // Query path; eg. "/cgi-bin/demo.py"
    Slice                queryParameters; // Query parameters; eg. "hello=world&abc=def"
    bool                 isLegacy;        // True when HTTP/1.0 instead of HTTP/1.1
    std::vector<Header>  headers;
    std::vector<uint8_t> body;

    /* Gets URL-decoded path of the request */
    inline std::string getPath() const
    {
        return queryPath.toString();
    }

    /* Sets the queryPath to newpath*/
    inline void setPath(std::string newpath)
    {
        queryPath = newpath;
    }

    /* Case-invariantly finds a header in the current request */
    inline const Header *findHeader(Slice key)
    {
        return findHeaderIn(headers, key);
    }

    /* Case-invariantly finds a header in the given vector of headers */
    static const Header *findHeaderIn(const std::vector<Header> &headers, Slice key);
};

#endif // HTTP_REQUEST_hpp

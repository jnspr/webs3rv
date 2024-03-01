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
    class Parser
    {
    public:
        /* Constructs a HTTP request parser using the given rules */
        Parser(const ServerConfig &config);

        /* Commits the given buffer to the current request
           Returns true if parsing was finished and moves the parsed request into `outRequest` */
        bool commit(const char *buffer, size_t length, HttpRequest &outRequest);
    };

    HttpMethod           method;
    std::string          query;           // Full query string; eg. "/cgi-bin/demo.py?hello=world&abc=def"
    Slice                queryPath;       // Query path; eg. "/cgi-bin/demo.py"
    Slice                queryParameters; // Query parameters; eg. "hello=world&abc=def"
    bool                 isLegacy;        // True when HTTP/1.0 instead of HTTP/1.1
    std::vector<uint8_t> body;
    size_t               bodylength;

    public:
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
};

#endif // HTTP_REQUEST_hpp

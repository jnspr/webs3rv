#ifndef HTTP_CONSTANTS_hpp
#define HTTP_CONSTANTS_hpp

#include "slice.hpp"

/* Enumeration of valid HTTP methods */
enum HttpMethod
{
    HTTP_METHOD_NONE,
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST,
    HTTP_METHOD_PUT,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_CONNECT,
    HTTP_METHOD_OPTIONS,
    HTTP_METHOD_TRACE,
    HTTP_METHOD_PATCH,
};

/* Parses an HTTP method from the given string slice */
HttpMethod parseHttpMethod(Slice slice);

/* Returns the name of a HTTP method as a constant string */
const char *httpMethodToString(HttpMethod method);

#endif // HTTP_CONSTANTS_hpp

#include "http_constants.hpp"

/* Parses an HTTP method from the given string slice */
HttpMethod parseHttpMethod(Slice slice)
{
    if (slice == C_SLICE("GET"))
        return HTTP_METHOD_GET;
    if (slice == C_SLICE("HEAD"))
        return HTTP_METHOD_HEAD;
    if (slice == C_SLICE("POST"))
        return HTTP_METHOD_POST;
    if (slice == C_SLICE("PUT"))
        return HTTP_METHOD_PUT;
    if (slice == C_SLICE("DELETE"))
        return HTTP_METHOD_DELETE;
    if (slice == C_SLICE("CONNECT"))
        return HTTP_METHOD_CONNECT;
    if (slice == C_SLICE("OPTIONS"))
        return HTTP_METHOD_OPTIONS;
    if (slice == C_SLICE("TRACE"))
        return HTTP_METHOD_TRACE;
    if (slice == C_SLICE("PATCH"))
        return HTTP_METHOD_PATCH;
    return HTTP_METHOD_NONE;
}

/* Returns the name of a HTTP method as a constant string */
const char *httpMethodToString(HttpMethod method)
{
    switch (method)
    {
        case HTTP_METHOD_GET:
            return "GET";
        case HTTP_METHOD_HEAD:
            return "HEAD";
        case HTTP_METHOD_POST:
            return "POST";
        case HTTP_METHOD_PUT:
            return "PUT";
        case HTTP_METHOD_DELETE:
            return "DELETE";
        case HTTP_METHOD_CONNECT:
            return "CONNECT";
        case HTTP_METHOD_OPTIONS:
            return "OPTIONS";
        case HTTP_METHOD_TRACE:
            return "TRACE";
        case HTTP_METHOD_PATCH:
            return "PATCH";
        default:
            return "NONE";
    }
}

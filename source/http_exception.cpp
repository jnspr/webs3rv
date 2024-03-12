#include "http_exception.hpp"

/* Constructs a parser exception using the given status code */
HttpException::HttpException(int statusCode)
    : _statusCode(statusCode)
{
}

/* Gets the cause of the error as a C-style string */
const char *HttpException::what() const throw()
{
    return "An error occurred while handling a HTTP request";
}

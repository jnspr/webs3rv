#include "http_exception.hpp"

/* Gets the cause of the error as a C-style string */
const char *HttpException::what() const throw()
{
    return "An error occurred while handling a HTTP request";
}

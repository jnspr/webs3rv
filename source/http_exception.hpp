#ifndef HTTP_EXCEPTION_hpp
#define HTTP_EXCEPTION_hpp

#include <stdexcept>

class HttpException: public std::exception
{
public:
    /* Constructs a parser exception using the given status code */
    HttpException(int statusCode);

    /* Gets the cause of the error as a C-style string */
    const char *what() const throw();

    /* Gets the status code */
    inline size_t getStatusCode() const
    {
        return _statusCode;
    }
private:
    int _statusCode;
};

#endif // HTTP_EXCEPTION_hpp

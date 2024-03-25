#ifndef HTTP_RESPONSE_hpp
#define HTTP_RESPONSE_hpp

#include <string>
#include <sstream>
#include <unistd.h>

#include "slice.hpp"

enum HttpResponseState
{
    HTTP_RESPONSE_UNINITIALIZED,
    HTTP_RESPONSE_INITIALIZED,
    HTTP_RESPONSE_FINALIZED
};

class HttpResponse
{
public:
    /* Constructs an uninitialized HTTP response */
    HttpResponse();
    
    /* Initialize the response class with bodyBuffer for cgi pages */
    void initialize(int statusCode, Slice statusMessage, const void *bodyBuffer, size_t bodySize);
    
    /* Initialize the response class with bodyFileno for static pages */
    void initialize(int statusCode, Slice statusMessage, int bodyFileno, size_t bodySize);
    
    /* Add a header field to the header response */
    void addHeader(Slice key, Slice value);

    /* Finalize Header */
    void finalizeHeader();
    
    /* Check if the response has data to send */
    bool hasData();

    /* Start transfer process of the response to the socket */
    void transferToSocket(int fileno);

    /* Gets the response's current state */
    inline HttpResponseState getState() const
    {
        return _state;
    }
private:
    HttpResponseState _state;
    std::stringstream _headerStream;
    std::string       _headerString;
    Slice             _headerSlice;
    Slice             _bodySlice;
    int               _bodyFileno;
    size_t            _bodyRemainder;
    char              _readBuffer[8192];

    /* Initializes the header string stream with a response line */
    void initializeHeader(int statusCode, Slice statusMessage, size_t bodySize);

    /* Attempts to send as many bytes as possible from a slice to a socket,
       only consumes the bytes that were actually sent */
    size_t sendSliceToSocket(int fileno, Slice &slice);

    /* Buffers and streams bytes out of `_bodyFileno` to the given socket */
    size_t streamFileToSocket(int fileno);
};

#endif // HTTP_RESPONSE_hpp

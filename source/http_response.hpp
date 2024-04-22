#ifndef HTTP_RESPONSE_hpp
#define HTTP_RESPONSE_hpp

#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdint.h>

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

    /* Initializes the response object with an empty body */
    inline void initializeEmpty(int statusCode, Slice statusMessage)
    {
        initializeUnowned(statusCode, statusMessage, Slice());
    }

    /* Initializes the response object with an owned string as body */
    void initializeOwned(int statusCode, Slice statusMessage, const std::string &body);

    /* Initializes the response object with a slice of unowned memory as body
       NOTE: The lifetime of this slice MUST match the response's */
    void initializeUnowned(int statusCode, Slice statusMessage, Slice body);

    /* Initializes the response object with a file stream of the given path */
    void initializeFileStream(int statusCode, Slice statusMessage, const char *path);

    /* Initializes the response object with CGI output data
       NOTE: The lifetime of this slice MUST match the response's */
    void initializeUnownedCgi(Slice response);

    /* Add a header field to the header response */
    void addHeader(Slice key, Slice value);

    /* Finalize Header */
    uint64_t finalizeHeader();

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
    std::string       _bodyBuffer;
    Slice             _headerSlice;
    Slice             _bodySlice;
    std::ifstream     _bodyStream;
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

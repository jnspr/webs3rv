#ifndef HTTP_RESPONSE_hpp
#define HTTP_RESPONSE_hpp

#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>

#include "slice.hpp"


class HttpResponse
{
public:
    HttpResponse();
    // Initialize the response class with bodyBuffer for cgi pages
    void initialize(int statusCode, Slice statusMessage, const void *bodyBuffer, size_t bodySize);
    // Initialize the response class with bodyFileno for static pages
    void initialize(int statusCode, Slice statusMessage, int bodyFileno, size_t bodySize);
    // Add a header field to the header response
    void addHeader(Slice key, Slice value);

    // Finalize Header
    void finalizeHeader();
    
    // Check if the response has data to send
    bool hasData();

    // Start transfer process of the response to the socket
    void transferToSocket(int fileno);
    
private:
    std::stringstream _headerStream;
    std::string       _headerString;
    size_t            _headerRemainder;
    int               _bodyFileno;
    const void       *_bodyBuffer;
    size_t            _bodyRemainder;
    char              _sendBuffer[8192];
    size_t            _sendBufferRemainder;

    // Send data to the socket file descriptor with bodyBuffer for cgi pages
    size_t sendToSocketFd(int socketFd, const void *buffer, size_t remainderLen);
    // Send data to the socket file descriptor with bodyFileno for static pages
    size_t sendToSocketFd(int socketFd, int fileFd, size_t remainderLen);
};

#endif // HTTP_RESPONSE_hpp

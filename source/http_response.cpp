
#include "http_response.hpp"

HttpResponse::HttpResponse()
    : _headerRemainder(0)
    , _bodyFileno(-1)
    , _bodyBuffer(NULL)
    , _bodyRemainder(0)
    , _sendBufferRemainder(0)
{
}

// Initialize the response class with bodyBuffer for cgi pages
void HttpResponse::initialize(int statusCode, Slice statusMessage, const void *bodyBuffer, size_t bodySize)
{
    _headerStream.str("");
    _headerStream.clear();
    _headerStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    _headerStream << "Content-Length: " << bodySize << "\r\n";
    _headerStream << "Connection: close\r\n";
    _bodyBuffer = bodyBuffer;
    _bodyRemainder = bodySize;
}

// Initialize the response class with bodyFileno for static pages
void HttpResponse::initialize(int statusCode, Slice statusMessage, int bodyFileno, size_t bodySize)
{
    _headerStream.str("");
    _headerStream.clear();
    _headerStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    _headerStream << "Content-Length: " << bodySize << "\r\n";
    _headerStream << "Connection: close\r\n";
    _bodyFileno = bodyFileno;
    _bodyRemainder = bodySize;
}

// Add a header field to the header response
void HttpResponse::addHeader(Slice key, Slice value)
{
    _headerStream << key << ": " << value << "\r\n";
}

// Finalize Header
void HttpResponse::finalizeHeader()
{
    _headerStream << "\r\n";
    _headerString = _headerStream.str();
    _headerRemainder = _headerString.size();
}

// Check if the response has data to send
bool HttpResponse::hasData()
{
    return _headerRemainder > 0 || _bodyRemainder > 0;
}

// Start transfer process of the response to the socket
void HttpResponse::transferToSocket(int fileno)
{
    if (_headerRemainder > 0)
        _headerRemainder -= sendToSocketFd(fileno, _headerString.c_str(), _headerRemainder);
    if (_bodyRemainder > 0)
    {
        if (_bodyFileno != -1)
            _bodyRemainder -= sendToSocketFd(fileno, _bodyBuffer, _bodyRemainder);
        else
            _bodyRemainder -= sendToSocketFd(fileno, _bodyFileno, _bodyRemainder);
    }
}

// Send data to the socket file descriptor with bodyBuffer for cgi pages
size_t HttpResponse::sendToSocketFd(int socketFd, const void *buffer, size_t remainderLen)
{
    ssize_t totalSend = 0;

    totalSend = send(socketFd, buffer, remainderLen, 0);
    if (totalSend == -1)
        throw std::runtime_error("Error sending data to socket");

    return static_cast<size_t>(totalSend);
}

// Send data to the socket file descriptor with bodyFileno for static pages
size_t HttpResponse::sendToSocketFd(int socketFd, int fileFd, size_t remainderLen)
{
    ssize_t bytesRead = 0;
    ssize_t totalSend = 0;

    if (_sendBufferRemainder == 0)
    {
        ssize_t bytesRead = read(fileFd, _sendBuffer, sizeof(_sendBuffer));
        if (bytesRead == -1)
            throw std::runtime_error("Error reading file");
        if (bytesRead == 0)
            return 0;
    }

    if (remainderLen - bytesRead < 0)
        throw std::runtime_error("File larger then expected");
    
    totalSend = send(socketFd, _sendBuffer, bytesRead, 0);
    if (totalSend == -1)
        throw std::runtime_error("Error sending data to socket");
    _sendBufferRemainder = bytesRead - totalSend;
        
    return static_cast<size_t>(totalSend);
}

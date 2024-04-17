#include "http_response.hpp"

#include <unistd.h>
#include <stdexcept>
#include <sys/socket.h>

/* Constructs an uninitialized HTTP response */
HttpResponse::HttpResponse()
    : _state(HTTP_RESPONSE_UNINITIALIZED)
{
}

/* Releases the response's resources */
HttpResponse::~HttpResponse()
{
    if ((_state == HTTP_RESPONSE_INITIALIZED || _state == HTTP_RESPONSE_FINALIZED) && _bodyFileno >= 0)
        close(_bodyFileno);
}

/* Initialize the response class with body */
void HttpResponse::initialize(int statusCode, Slice statusMessage, const std::string &body)
{
    initializeHeader(statusCode, statusMessage, body.size());
    _bodyBuffer    = body;
    _bodySlice     = Slice(_bodyBuffer);
    _bodyFileno    = -1;
    _bodyRemainder = body.size();
    _state         = HTTP_RESPONSE_INITIALIZED;
}

/* Initialize the response class with bodyBuffer for cgi pages */
void HttpResponse::initialize(int statusCode, Slice statusMessage, const void *bodyBuffer, size_t bodySize)
{
    initializeHeader(statusCode, statusMessage, bodySize);
    _bodySlice = Slice(static_cast<const char *>(bodyBuffer), bodySize);
    _bodyFileno = -1;
    _bodyRemainder = bodySize;
    _state = HTTP_RESPONSE_INITIALIZED;
}

/* Initialize the response class with bodyFileno for static pages (takes ownership of the file descriptor) */
void HttpResponse::initialize(int statusCode, Slice statusMessage, int bodyFileno, size_t bodySize)
{
    try
    {
        if (bodyFileno < 0)
            throw std::logic_error("Attempt to initialize HTTP response with bad file descriptor");
        initializeHeader(statusCode, statusMessage, bodySize);
        _bodySlice = Slice();
        _bodyFileno = bodyFileno;
        _bodyRemainder = bodySize;
        _state = HTTP_RESPONSE_INITIALIZED;
    }
    catch (...)
    {
        close(bodyFileno);
        throw;
    }
}

/* Add a header field to the header response */
void HttpResponse::addHeader(Slice key, Slice value)
{
    if (_state != HTTP_RESPONSE_INITIALIZED)
        throw std::logic_error("addHeader() called on uninitialized response");
    _headerStream << key << ": " << value << "\r\n";
}

/* Finalize Header */
uint64_t HttpResponse::finalizeHeader()
{
    if (_state != HTTP_RESPONSE_INITIALIZED)
        throw std::logic_error("finalizeHeader() called on uninitialized response");
    _headerStream << "\r\n";
    _headerString = _headerStream.str();
    _headerSlice = _headerString;
    _state = HTTP_RESPONSE_FINALIZED;

    /* Returns the relative timeout for the client in seconds */
    double timeout = static_cast<double>(this->_bodyRemainder) / (1024.0 * 1024.0);
    if (timeout < 1000)
        return 1000;
    return static_cast<uint64_t>(timeout);
}

/* Check if the response has data to send */
bool HttpResponse::hasData()
{
    if (_state != HTTP_RESPONSE_FINALIZED)
        return false;
    return !_headerSlice.isEmpty() || _bodyRemainder > 0;
}

// Start transfer process of the response to the socket
void HttpResponse::transferToSocket(int fileno)
{
    if (_state != HTTP_RESPONSE_FINALIZED)
        throw std::logic_error("transferToSocket() called on non-finalized response");

    // Send the header first
    if (!_headerSlice.isEmpty())
    {
        sendSliceToSocket(fileno, _headerSlice);
        return;
    }

    // Send the body
    if (_bodyRemainder > 0)
    {
        size_t bytesSent;
        if (_bodyFileno == -1)
            bytesSent = sendSliceToSocket(fileno, _bodySlice);
        else
            bytesSent = streamFileToSocket(fileno);

        // Reduce the number of remaining bytes but check for underflow first
        if (bytesSent > _bodyRemainder)
            bytesSent = _bodyRemainder;
        _bodyRemainder -= bytesSent;
    }
}

/* Initializes the header string stream with a response line */
void HttpResponse::initializeHeader(int statusCode, Slice statusMessage, size_t bodySize)
{
    _headerStream.clear();
    _headerStream << "HTTP/1.1 " << statusCode << ' ' << statusMessage << "\r\n"
                  << "Content-Length: " << bodySize << "\r\n"
                  << "Connection: close\r\n";
}

/* Attempts to send as many bytes as possible from a slice to a socket,
   only consumes the bytes that were actually sent */
size_t HttpResponse::sendSliceToSocket(int fileno, Slice &slice)
{
    ssize_t result;

    result = send(fileno, &slice[0], slice.getLength(), 0);
    if (result == -1)
        throw std::runtime_error("Unable to send data to socket");
    if (result == 0)
        throw std::runtime_error("Remote host has closed the connection");

    size_t bytesSent = static_cast<size_t>(result);
    slice.consumeStart(bytesSent);
    return bytesSent;
}


/* Buffers and streams bytes out of `_bodyFileno` to the given socket */
size_t HttpResponse::streamFileToSocket(int fileno)
{
    if (_bodySlice.isEmpty())
    {
        ssize_t bytesRead = read(_bodyFileno, _readBuffer, sizeof(_readBuffer));
        if (bytesRead == -1)
            throw std::runtime_error("Unable to read file");
        if (bytesRead == 0)
            throw std::runtime_error("Unexpected end of file");
        _bodySlice = Slice(_readBuffer, static_cast<size_t>(bytesRead));
    }
    return sendSliceToSocket(fileno, _bodySlice);
}

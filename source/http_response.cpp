#include "http_response.hpp"
#include "http_exception.hpp"

#include <unistd.h>
#include <stdexcept>
#include <sys/socket.h>

/* Constructs an uninitialized HTTP response */
HttpResponse::HttpResponse()
    : _state(HTTP_RESPONSE_UNINITIALIZED)
{
}

/* Initializes the response object with an owned string */
void HttpResponse::initializeOwned(int statusCode, Slice statusMessage, const std::string &body)
{
    initializeHeader(statusCode, statusMessage, body.size());
    _bodyBuffer    = body;
    _bodySlice     = Slice(_bodyBuffer);
    _bodyRemainder = body.size();
    _state         = HTTP_RESPONSE_INITIALIZED;
}

/* Initializes the response object with a slice of unowned memory
   NOTE: The lifetime of this slice MUST match the response's */
void HttpResponse::initializeUnowned(int statusCode, Slice statusMessage, Slice body)
{
    initializeHeader(statusCode, statusMessage, body.getLength());
    _bodySlice     = body;
    _bodyRemainder = body.getLength();
    _state = HTTP_RESPONSE_INITIALIZED;
}

/* Initialize the response object with a file stream of the given path */
void HttpResponse::initializeFileStream(int statusCode, Slice statusMessage, const char *path)
{
    // Open the file stream at the file's end to obtain its length
    _bodyStream.open(path, std::ios::binary | std::ios::ate);
    if (!_bodyStream.is_open())
        throw HttpException(500);
    size_t length = _bodyStream.tellg();

    // Seek back to the start
    _bodyStream.seekg(0);
    if (!_bodyStream.good())
        throw HttpException(500);

    initializeHeader(statusCode, statusMessage, length);
    _bodySlice     = Slice();
    _bodyRemainder = length;
    _state         = HTTP_RESPONSE_INITIALIZED;
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

    // This is an approximation based on very slow network speed
    // A GiB of data can take up to ~14 hours before the client is dropped
    // A MiB of data can take up to ~50 seconds before the client is dropped
    // Anything below or equal to 20 KiB is clamped up to a second
    double timeout = static_cast<double>(this->_bodyRemainder) * 0.05;
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
        if (_bodyStream.is_open())
            bytesSent = streamFileToSocket(fileno);
        else
            bytesSent = sendSliceToSocket(fileno, _bodySlice);

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
        _bodyStream.read(_readBuffer, sizeof(_readBuffer));
        if (_bodyStream.bad())
            throw std::runtime_error("Unable to read file");
        size_t bytesRead = _bodyStream.gcount();
        if (bytesRead == 0)
            throw std::runtime_error("Unexpected end of file");
        _bodySlice = Slice(_readBuffer, bytesRead);
    }
    return sendSliceToSocket(fileno, _bodySlice);
}

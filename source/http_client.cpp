#include "http_client.hpp"
#include "application.hpp"

#include <stdio.h>
#include <unistd.h>

/* Constructs a HTTP client using the given socket file descriptor */
HttpClient::HttpClient(Application &application, const ServerConfig &config, int fileno, uint64_t timeoutStart)
    : _application(application)
    , _config(config)
    , _fileno(fileno)
    , _timeoutStart(timeoutStart)
    , _markedForCleanup(false)
    , _parser(config)
    , _process(NULL)
{
}

/* Closes the client's file descriptor */
HttpClient::~HttpClient()
{
    if (_process != NULL)
        delete _process;
    close(_fileno);
}

/* Handles one or multiple events */
void HttpClient::handleEvents(uint32_t eventMask)
{
    if (_markedForCleanup)
        return;

    if (eventMask & EPOLLHUP)
        throw std::runtime_error("End of stream");

    if (eventMask & EPOLLIN)
    {
        ssize_t length;
        char    buffer[8192];

        if ((length = read(_fileno, buffer, sizeof(buffer))) < 0)
            throw std::runtime_error("Unable to read from client");
        if (length == 0)
            throw std::runtime_error("End of stream");

        HttpRequest request;
        if (_parser.commit(buffer, length, request))
        {
        }
    }
}

/* Handles an exception that occurred in `handleEvent()` */
void HttpClient::handleException()
{
    if (_markedForCleanup)
        return;

    _cleanupNext = _application._cleanupClients;
    _application._cleanupClients = this;
    _markedForCleanup = true;
}

void HttpClient::handleCgiState()
{
    if (_process == NULL)
        return;
    // TODO: Implement this
}

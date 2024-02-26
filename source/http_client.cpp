#include "http_client.hpp"
#include "application.hpp"

#include <stdio.h>
#include <unistd.h>

/* Constructs a HTTP client using the given socket file descriptor */
HttpClient::HttpClient(Application &application, const ServerConfig &config, int fileno, time_t timeoutStart)
    : _application(application)
    , _config(config)
    , _fileno(fileno)
    , _timeoutStart(timeoutStart)
    , _markedForCleanup(false)
    , _parser(config)
{
}

/* Closes the client's file descriptor */
HttpClient::~HttpClient()
{
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

/* Finds a matching route for path and returns it in a struct*/
RouteResult ServerConfig::findRoute(Slice path) const
{
    RouteResult result = {};
    size_t biggestleng = 0;
    result.wasFound = 0;

    for (size_t i = 0; i < localRoutes.size(); i++)
    {
        if (path.startsWith(localRoutes[i].path))
        {
            if (localRoutes[i].path.size() > biggestleng)
            {
                result.wasFound = true;
                result.localRoute = &localRoutes[i];
                result.isRedirect = false;
                result.path = localRoutes[i].path;
                result.redirectRoute = NULL;
            }
        }
    }
    
    if (result.wasFound == true)
       return result;

        for (size_t i = 0; i < redirectRoutes.size(); i++)
    {
        if (path.startsWith(redirectRoutes[i].path))
        {
            if (redirectRoutes[i].path.size() > biggestleng)
            {
                result.wasFound = true;
                result.redirectRoute = &redirectRoutes[i];
                result.isRedirect = true;
                result.path = redirectRoutes[i].path;
                result.localRoute = NULL;
            }
        }
    }    
    return result;
}

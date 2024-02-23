#include "http_client.hpp"
#include "application.hpp"

#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include "config.hpp"
#include <algorithm>

/* Constructs a HTTP client using the given socket file descriptor */
HttpClient::HttpClient(Application &application, const ServerConfig &config, int fileno, time_t timeoutStart)
    : _application(application)
    , _config(config)
    , _fileno(fileno)
    , _timeoutStart(timeoutStart)
    , _markedForCleanup(false)
    //, _parser(config)
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
        if (true)
        //parser.commit(buffer, length, request) 
        {
            request.queryPath = C_SLICE("/cgi-bin/demo.py"); // dummy for testing
            // dummy for testing
            std::ostringstream oss;
            oss << request.queryPath;
            std::string s = oss.str();
            RouteResult result = _config.findRoute(s);
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

RouteResult ServerConfig::findRoute(const std::string &path) const
{
    RouteResult result = {};
    Slice       pathSlice(path.c_str(), path.length());
    Slice      remainder; // how to free this
    
    std::cout << "pathslice first: " << pathSlice << std::endl;
    // search for the route
    
    while (true)
    {
        std::vector<LocalRouteConfig>::const_iterator it = std::find(localRoutes.begin(), localRoutes.end(), pathSlice.toString());
        if (it != localRoutes.end())
        {
            result.wasFound = true;
            result.localRoute = &(*it);
            result.isRedirect = false;
            result.path = pathSlice.toString();
            result.redirectRoute = NULL;
            return result;
        }
        std::vector<RedirectRouteConfig>::const_iterator it_1 = std::find(redirectRoutes.begin(), redirectRoutes.end(), pathSlice.toString());
        if (it_1 != redirectRoutes.end())
        {
            result.wasFound = true;
            result.redirectRoute = &(*it_1);
            result.isRedirect = true;
            result.path = pathSlice.toString();
            result.localRoute = NULL;
            return result;
        }
        // no route found slice to the last slash
        pathSlice.splitEnd('/', remainder); 

        std::cout << "afterslice: " << pathSlice << std::endl;
        std::cout << "remainder: " << remainder << std::endl;
        if (remainder.isEmpty())
        {
            std::cout << "empty return isfalse here" << std::endl;
            break;
        }
    }
}

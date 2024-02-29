#include "cgi_process.hpp"
#include "http_client.hpp"

#include <unistd.h>

/* Constructs a CGI process from the given client, request and route result */
CgiProcess::CgiProcess(HttpClient *client, const HttpRequest &request, const RouteResult &routeResult)
    : _process(setupArguments(request, routeResult), setupEnvironment(request, routeResult))
{
    (void)client;
    (void)request;
    (void)routeResult;
}

/* Handles one or multiple events */
void CgiProcess::handleEvents(uint32_t eventMask)
{
    if (_state != CGI_PROCESS_RUNNING)
        return;
    (void)eventMask;
    // TODO: Implement this
}

/* Handles an exception that occurred in `handleEvent()` */
void CgiProcess::handleException()
{
    if (_state != CGI_PROCESS_RUNNING)
        return;
    _state = CGI_PROCESS_FAILURE;
    _client->handleCgiState();
}

/* Creates a vector of strings for the process arguments */
std::vector<std::string> CgiProcess::setupArguments(const HttpRequest &request, const RouteResult &routeResult)
{
    (void)request;
    (void)routeResult;
    std::vector<std::string> result;
    // TODO: Implement this

    /* Extract the file name from the route */
    std::string fileName;
    if (routeResult.isRedirect)
    {
        std::string path = routeResult.redirectRoute->path;
        size_t pos = path.rfind('/');
        if (pos != std::string::npos)
            fileName = path.substr(pos + 1);
    }
    else
    {
        std::string path = routeResult.localRoute->path;
        size_t pos = path.rfind('/');
        if (pos != std::string::npos)
            fileName = path.substr(pos + 1);
    }
    result.push_back(fileName);        

    return result;
}

/* Creates a vector of strings for the process environment */
std::vector<std::string> CgiProcess::setupEnvironment(const HttpRequest &request, const RouteResult &routeResult)
{
    (void)request;
    (void)routeResult;
    std::vector<std::string> result;
    // TODO: Implement this

    /* Add the environment variables C++98 */
    result.push_back("GATEWAY_INTERFACE=CGI/1.1");
    result.push_back("SERVER_SOFTWARE=JPwebs3rv/1.0");
    result.push_back("SERVER_NAME=" + numberToString(routeResult.serverConfig->host));
    if (request.isLegacy)
        result.push_back("SERVER_PROTOCOL=HTTP/1.0");
    else
        result.push_back("SERVER_PROTOCOL=HTTP/1.1");
    result.push_back("SERVER_PORT=" + numberToString(routeResult.serverConfig->port));
    result.push_back("REQUEST_METHOD=" + std::string(httpMethodToString(request.method)));
    result.push_back("CONTENT_TYPE=");
    result.push_back("CONTENT_LENGTH=" + numberToString(request.body.size()));
    result.push_back("SCRIPT_NAME=");
    result.push_back("PATH_INFO=");
    result.push_back("PATH_TRANSLATED=");
    result.push_back("QUERY_STRING=" + request.queryParameters.toString());
    result.push_back("REMOTE_HOST=");
    result.push_back("REMOTE_ADDR=");

    return result;
}

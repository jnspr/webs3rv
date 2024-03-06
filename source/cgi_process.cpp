#include "cgi_process.hpp"
#include "http_client.hpp"

#include <unistd.h>

/* Constructs a CGI process from the given client, request and route result */
CgiProcess::CgiProcess(HttpClient *client, const HttpRequest &request, const RoutingInfo &routingInfo)
    : _process(setupArguments(request, routingInfo), setupEnvironment(request, routingInfo))
{
    (void)client;
    (void)request;
    (void)routingInfo;
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
std::vector<std::string> CgiProcess::setupArguments(const HttpRequest &request, const RoutingInfo &routingInfo)
{
    (void)request;
    (void)routingInfo;
    std::vector<std::string> result;
    // TODO: Implement this

    /* Extract the file name from the route */
    std::string fileName;
    switch (routingInfo.status)
    {
        case ROUTING_STATUS_FOUND_LOCAL:
        {
            std::string path = routingInfo.getRedirectRoute()->path;
            size_t pos = path.rfind('/');
            if (pos != std::string::npos)
                fileName = path.substr(pos + 1);
        }
        break;
        case ROUTING_STATUS_FOUND_REDIRECT:
        {
            std::string path = routingInfo.getLocalRoute()->path;
            size_t pos = path.rfind('/');
            if (pos != std::string::npos)
                fileName = path.substr(pos + 1);
        }
        break;
        default:
            throw std::logic_error("Invalid routing info passed to CGI process");
    }
    result.push_back(fileName);
    return result;
}

/* Creates a vector of strings for the process environment */
std::vector<std::string> CgiProcess::setupEnvironment(const HttpRequest &request, const RoutingInfo &routingInfo)
{
    (void)request;
    (void)routingInfo;
    std::vector<std::string> result;
    // TODO: Implement this

    result.push_back("GATEWAY_INTERFACE=CGI/1.1");
    result.push_back("SERVER_SOFTWARE=JPwebs3rv/1.0");
    result.push_back("SERVER_NAME=" + Utility::numberToString(routingInfo.serverConfig->host));
    if (request.isLegacy)
        result.push_back("SERVER_PROTOCOL=HTTP/1.0");
    else
        result.push_back("SERVER_PROTOCOL=HTTP/1.1");
    result.push_back("SERVER_PORT=" + Utility::numberToString(routingInfo.serverConfig->port));
    result.push_back("REQUEST_METHOD=" + std::string(httpMethodToString(request.method)));
    result.push_back("CONTENT_TYPE=");
    result.push_back("CONTENT_LENGTH=" + Utility::numberToString(request.body.size()));
    /*The path is virtual because it represents a URL path rather than a physical file system path. Is request.queryPath the virtual path?*/
    result.push_back("SCRIPT_NAME=" + request.queryPath.toString());
    result.push_back("PATH_INFO=" + request.queryParameters.toString());
    if (request.queryParameters.isEmpty())
        result.push_back("PATH_TRANSLATED=NULL");
    else
        result.push_back("PATH_TRANSLATED=" + routingInfo.getRedirectRoute()->redirectLocation + request.queryParameters.toString());
    result.push_back("QUERY_STRING=" + request.queryParameters.toString());
    /* Will not be used and no DNS lookup performed*/
    result.push_back("REMOTE_HOST=NULL");
    result.push_back("REMOTE_ADDR=" + Utility::numberToString(request.clientHost));

    return result;
}

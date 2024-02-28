#include "cgi_process.hpp"
#include "http_client.hpp"

#include <unistd.h>

/* Constructs a CGI process from the given client, request and route result */
CgiProcess::CgiProcess(HttpClient *client, const HttpRequest &request, const RouteResult &routeResult, uint64_t timeoutStart)
    : _process(setupArguments(request, routeResult), setupEnvironment(request, routeResult))
    , _timeoutStart(timeoutStart)
{
    (void)client;
    (void)request;
    (void)routeResult;
    (void)_timeoutStart;
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
    return result;
}

/* Creates a vector of strings for the process environment */
std::vector<std::string> CgiProcess::setupEnvironment(const HttpRequest &request, const RouteResult &routeResult)
{
    (void)request;
    (void)routeResult;
    std::vector<std::string> result;
    // TODO: Implement this
    return result;
}

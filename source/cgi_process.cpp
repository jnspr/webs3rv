#include "cgi_process.hpp"
#include "http_client.hpp"
#include "slice.hpp"
#include "application.hpp"
#include <string.h>

#include <unistd.h>

CgiPathInfo::CgiPathInfo(const std::string &nodePath)
{
    // Split script file and directory
    Slice scriptFileSlice;
    Slice scriptDirectorySlice(nodePath);
    if (!scriptDirectorySlice.splitEnd('/', scriptFileSlice))
        throw std::runtime_error("Unable to split script directory");

    // Populate the structure
    fileName = scriptFileSlice.toString();
    workingDirectory = scriptDirectorySlice.toString();
}

/* Constructs a CGI process from the given client, request and route result */
CgiProcess::CgiProcess(HttpClient *client, const HttpRequest &request, const RoutingInfo &routingInfo)
    : _state(CGI_PROCESS_RUNNING)
    , _pathInfo(routingInfo.nodePath)
    , _client(client)
    , _process(setupArguments(request, routingInfo, _pathInfo.fileName), setupEnvironment(request, routingInfo)
    , _pathInfo.workingDirectory)
    , _timeout(TIMEOUT_CGI_MS)
    , _bodyOffset(0)
    , _isInOutputPhase(false)
{
}

/* Handles one or multiple events */
void CgiProcess::handleEvents(uint32_t eventMask)
{
    if (_state != CGI_PROCESS_RUNNING)
        return;

    if (eventMask & EPOLLOUT)
    {
        const std::vector<uint8_t> &requestBody = _client->_parser.getRequest().body;
        ssize_t result = write(_process.getInputFileno(), &requestBody[_bodyOffset], requestBody.size() - _bodyOffset);
        if (result < 0)
            throw std::runtime_error("Unable to write to CGI process");
        size_t writeLength = static_cast<size_t>(result);
        _bodyOffset += writeLength;

        // switch to output phase
        // TODO(jnspr): check exception safety etc etc
        _client->_application._dispatcher.unsubscribe(_process.getInputFileno());
        _process.closeInput();
        _isInOutputPhase = true;

        _client->_application._dispatcher.subscribe(_process.getOutputFileno(), EPOLLIN | EPOLLHUP, this);
        return;
    }
    if (getProcess().getStatus() == PROCESS_EXIT_SUCCESS)
    {
        _state = CGI_PROCESS_SUCCESS;
        _client->handleCgiState();
    }
    else if (getProcess().getStatus() == PROCESS_EXIT_FAILURE)
    {
        _state = CGI_PROCESS_FAILURE;
        _client->handleCgiState();
    }
    else if (getProcess().getStatus() == PROCESS_RUNNING)
    {
        char data[8192];
        ssize_t result = read(getProcess().getOutputFileno(), data, sizeof(data));
        if (result < 0)
            throw std::runtime_error("Unable to read from CGI process");

        // Copy the bytes into the body buffer
        size_t copyLength = static_cast<size_t>(result);
        size_t oldLength = _buffer.size();
        if (SIZE_MAX - oldLength < copyLength)
            throw std::runtime_error("Body exeed from CGI process");
        _buffer.resize(oldLength + copyLength);
        memcpy(&_buffer[oldLength], &data[0], copyLength);
    }
}

/* Handles an exception that occurred in `handleEvent()` */
void CgiProcess::handleException(const char *message)
{
    (void)message;
    if (_state != CGI_PROCESS_RUNNING)
        return;
    _state = CGI_PROCESS_FAILURE;
    _client->handleCgiState();
}

/* Creates a vector of strings for the process arguments */
std::vector<std::string> CgiProcess::setupArguments(const HttpRequest &request, const RoutingInfo &routingInfo, const std::string &fileName)
{
    (void)request;
    std::vector<std::string> result;
    result.push_back(routingInfo.cgiInterpreter);
    result.push_back(fileName);
    return result;
}

/* Creates a vector of strings for the process environment */
std::vector<std::string> CgiProcess::setupEnvironment(const HttpRequest &request, const RoutingInfo &routingInfo)
{
    std::vector<std::string> result;

    result.push_back("GATEWAY_INTERFACE=CGI/1.1");
    result.push_back("SERVER_SOFTWARE=JPwebs3rv/1.0");
    result.push_back("SERVER_NAME=" + Utility::numberToString(routingInfo.serverConfig->host));
    if (request.isLegacy)
        result.push_back("SERVER_PROTOCOL=HTTP/1.0");
    else
        result.push_back("SERVER_PROTOCOL=HTTP/1.1");
    result.push_back("SERVER_PORT=" + Utility::numberToString(routingInfo.serverConfig->port));
    result.push_back("REQUEST_METHOD=" + std::string(httpMethodToString(request.method)));
    const HttpRequest::Header *contentType = request.findHeader(C_SLICE("Content-Type"));
    if (contentType == NULL)
        result.push_back("CONTENT_TYPE=NULL");
    else
        result.push_back("CONTENT_TYPE=" + contentType->getValue());
    result.push_back("CONTENT_LENGTH=" + Utility::numberToString(request.body.size()));
    result.push_back("SCRIPT_NAME=" + request.queryPath);
    result.push_back("PATH_INFO=");
    if (request.queryParameters.isEmpty())
        result.push_back("PATH_TRANSLATED=NULL");
    else
        result.push_back("PATH_TRANSLATED=" + routingInfo.nodePath + request.queryParameters.toString());
    result.push_back("QUERY_STRING=?" + request.queryParameters.toString());
    /* Will not be used and no DNS lookup performed*/
    result.push_back("REMOTE_HOST=NULL");
    result.push_back("REMOTE_ADDR=" + Utility::numberToString(request.clientHost));

    return result;
}

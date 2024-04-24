#include "http_exception.hpp"
#include "debug_utility.hpp"
#include "http_client.hpp"
#include "application.hpp"
#include "utility.hpp"
#include "mime_db.hpp"
#include "error_db.hpp"
#include "html_generator.hpp"
#include "config.hpp"
#include "upload_handler.hpp"

#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>

/* Constructs a HTTP client using the given socket file descriptor */
HttpClient::HttpClient(Application &application, const ServerConfig *config, int fileno, uint32_t host, uint16_t port)
    : _application(application)
    , _config(config)
    , _fileno(fileno)
    , _timeout(TIMEOUT_REQUEST_MS)
    , _waitingForClose(false)
    , _markedForCleanup(false)
    , _process(NULL)
    , _host(host)
    , _port(port)
    , _parser(*config, host, port)
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
        char buffer[8192];

        if ((length = read(_fileno, buffer, sizeof(buffer))) < 0)
            throw std::runtime_error("Unable to read from client");

        if (_waitingForClose)
        {
            markForCleanup();
            return;
        }

        if (length == 0)
            throw std::runtime_error("End of stream");

        Slice data(buffer, length);
        try
        {
            if (_parser.commit(data))
            {
                switch (_parser.getPhase())
                {
                case HTTP_REQUEST_HEADER_EXCEED:
                    throw HttpException(413);
                case HTTP_REQUEST_BODY_EXCEED:
                    throw HttpException(413);
                case HTTP_REQUEST_MALFORMED:
                    throw HttpException(400);
                case HTTP_REQUEST_COMPLETED:
                {
                    // Adjust the server configuration to match the requested server by its host, taking the first one if not found
                    const HttpRequest::Header *host = _parser.getRequest().findHeader(C_SLICE("Host"));
                    if (host != NULL)
                    {
                        Slice serverName = host->getValue();
                        Slice port;
                        serverName.splitEnd(':', port);
                        (void)port;
                        _config = _config->findServer(serverName);
                    }
                    handleRequest(_parser.getRequest());
                }
                default:
                    break;
                }
            }
        } catch (HttpException &exception)
        {
            createErrorResponse(exception.getStatusCode());
        }
#ifdef __42_LIKES_WASTING_CPU_CYCLES__
        return;
#endif // __42_LIKES_WASTING_CPU_CYCLES__
    }

    if (eventMask & EPOLLOUT)
    {
        if (_response.hasData())
            _response.transferToSocket(_fileno);
        if (!_response.hasData())
        {
            // Do not directly close the connection after sending the response
            // Switch back to read events and wait for the client to close the connection in
            // and set a timeout so it doesn't linger
            _timeout = Timeout(TIMEOUT_CLOSING_MS);
            _application._dispatcher.modify(_fileno, EPOLLIN | EPOLLHUP, this);
            _waitingForClose = true;
        }
    }
}

void HttpClient::handleRequest(const HttpRequest &request)
{
    if (!Utility::checkPathLevel(request.queryPath))
        throw std::runtime_error("Client tried to access above-root directory");

    RoutingInfo info = info.findRoute(*_config, request.queryPath);

    // HACK: For reusing the existing handling logic when the path must be changed
repeat:
    if (info.status == ROUTING_STATUS_NOT_FOUND)
        throw HttpException(404);
    else if (info.status == ROUTING_STATUS_NO_ACCESS)
        throw HttpException(403);
    else if (info.status == ROUTING_STATUS_FOUND_LOCAL)
    {
        std::set<HttpMethod>::iterator it = info.getLocalRoute()->allowedMethods.find(request.method);
        if (it == info.getLocalRoute()->allowedMethods.end())
            throw HttpException(405);

        switch (info.getLocalNodeType())
        {
        case NODE_TYPE_REGULAR:
            if (info.hasCgiInterpreter)
            {
                _application.startCgiProcess(this, request, info);
                _timeout.stop();
            }
            else if (request.method == HTTP_METHOD_DELETE)
            {
                 if (unlink(info.nodePath.c_str()) == -1)
                    throw HttpException(403);
                _response.initializeEmpty(204, C_SLICE("No Content"));
                _timeout = Timeout(_response.finalizeHeader());
            }
            else
            {
                setupFileResponse(200, C_SLICE("OK"), info.nodePath);
            }
            break;
        case NODE_TYPE_DIRECTORY:
            if (request.method == HTTP_METHOD_POST)
            {
                if (info.getLocalRoute()->allowUpload)
                {
                    UploadHandler::handleUpload(request, info);

                    // Redirect the client to the upload directory
                    _response.initializeEmpty(303, C_SLICE("See Other"));
                    _response.addHeader(C_SLICE("Location"), request.queryPath);
                    _response.finalizeHeader();
                }
                else
                    throw HttpException(403);
            }
            else if (!Slice(request.queryPath).endsWith(C_SLICE("/")))
            {
                _response.initializeEmpty(301, C_SLICE("Moved Permanently"));
                _response.addHeader(C_SLICE("Location"), Slice(request.queryPath + "/"));
                _timeout = Timeout(_response.finalizeHeader());
            }
            else if (!info.getLocalRoute()->indexFile.empty())
            {
                // HACK: Temporary solution for directory index access, refactor after the
                //       whole handling logic is done
                std::string newPath = request.queryPath + '/' + info.getLocalRoute()->indexFile;
                info = RoutingInfo::findRoute(*_config, newPath);
                // HACK: Prevent infinite loop on misconfigured server
                if (info.status != ROUTING_STATUS_FOUND_LOCAL || info.getLocalNodeType() != NODE_TYPE_DIRECTORY)
                    goto repeat;
                throw HttpException(500);
            }
            else if (info.getLocalRoute()->allowListing)
            {
                _response.initializeOwned(200, C_SLICE("OK"), HtmlGenerator::directoryList(info.nodePath.c_str()));
                _response.addHeader(C_SLICE("Content-Type"), C_SLICE("text/html"));
                _timeout = Timeout(_response.finalizeHeader());
            }
            else
                throw HttpException(403);
            break;
        default:
            break;
        }
    }
    else if (info.status == ROUTING_STATUS_FOUND_REDIRECT)
    {
        std::set<HttpMethod>::iterator it = info.getRedirectRoute()->allowedMethods.find(request.method);
        if (it == info.getRedirectRoute()->allowedMethods.end())
            throw HttpException(405);

        Slice routeRelativeQuery = Slice(request.query)
            .cut(info.getRedirectRoute()->path.size());
        routeRelativeQuery.consumeStart(C_SLICE("/"));

        Slice rewritePrefix = Slice(info.getRedirectRoute()->redirectLocation)
            .stripEnd('/');

        _response.initializeEmpty(307, C_SLICE("Temporary Redirect"));
        _response.addHeader(C_SLICE("Location"),  rewritePrefix.toString() + '/' + routeRelativeQuery.toString());
        _timeout = Timeout(_response.finalizeHeader());
    }

    if (_response.getState() != HTTP_RESPONSE_FINALIZED && _process == NULL)
        throw HttpException(500);
    if (_process == NULL)
        _application._dispatcher.modify(_fileno, EPOLLOUT | EPOLLHUP, this);
}

void HttpClient::setupFileResponse(size_t statusCode, Slice statusMessage, const std::string &path)
{
    std::string mimeType = g_mimeDB.getMimeType(path);

    // Setup a file stream response
    _response.initializeFileStream(statusCode, statusMessage, path.c_str());
    _response.addHeader(C_SLICE("Content-Type"), mimeType);
    _timeout = Timeout(_response.finalizeHeader());
}

/* Handles an exception that occurred in `handleEvent()` */
void HttpClient::handleException(const char *message)
{
    (void)message;
    //std::cout << "Exception while handling HTTP client event: " << message << std::endl;
    markForCleanup();
}

void HttpClient::handleCgiState()
{
    if (_process == NULL)
        return;
    switch (_process->getState())
    {
        case CGI_PROCESS_RUNNING:
            break;
        case CGI_PROCESS_SUCCESS:
        {
            _response.initializeUnownedCgi(Slice(_process->_buffer));
            _timeout = Timeout(_response.finalizeHeader());
            _application._dispatcher.unsubscribe(_process->getProcess().getOutputFileno());
            _process->_subscribeFlags &= ~SUBSCRIBE_FLAG_OUTPUT;
            _application._dispatcher.modify(_fileno, EPOLLOUT | EPOLLHUP, this);
        }
        break;
        case CGI_PROCESS_FAILURE:
            _application.closeCgiProcess(this);
            createErrorResponse(502);
            break;
        case CGI_PROCESS_TIMEOUT:
            _application.closeCgiProcess(this);
            createErrorResponse(504);
            break;
    }
}

/* Marks the client to be cleaned up during the next cleanup cycle */
void HttpClient::markForCleanup()
{
    if (_markedForCleanup)
        return;

    _cleanupNext = _application._cleanupClients;
    _application._cleanupClients = this;
    _markedForCleanup = true;
}

/* Create error response */
void HttpClient::createErrorResponse(size_t statusCode)
{
    bool alreadyHandled = false;

    // Check if for the error code was a static error page defined in the config file
    std::map<int, std::string>::const_iterator findResult = _config->errorPages.find(statusCode);
    if (findResult != _config->errorPages.end())
    {
        try
        {
            setupFileResponse(statusCode, g_errorDB.getErrorType(statusCode), findResult->second);
            alreadyHandled = true;
        }
        catch (...)
        {
        }
    }

    if (!alreadyHandled)
    {
        // Find the error message
        const std::string &errorMessage = g_errorDB.getErrorType(statusCode);
        std::string errorPage = HtmlGenerator::errorPage(statusCode);

        // Build the response and set its timeout
        _response.initializeOwned(statusCode, errorMessage, errorPage);
        _response.addHeader(C_SLICE("Content-Type"), C_SLICE("text/html"));
        _timeout = Timeout(_response.finalizeHeader());
    }

    // Switch the dispatcher to POLLOUT
    _application._dispatcher.modify(_fileno, EPOLLOUT | EPOLLHUP, this);
}

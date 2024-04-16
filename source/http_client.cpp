#include "http_exception.hpp"
#include "debug_utility.hpp"
#include "http_client.hpp"
#include "application.hpp"
#include "utility.hpp"
#include "mime_db.hpp"
#include "error_db.hpp"
#include "html_generator.hpp"
#include "config.hpp"

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
                        _config = _config->findServer(host->getValue());
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
            }
            else if (request.method == HTTP_METHOD_DELETE)
            {
                 if (unlink(info.nodePath.c_str()) == -1)
                    throw HttpException(403);
                _response.initialize(204, C_SLICE("No Content"), "", 0);
                _timeout = Timeout(_response.finalizeHeader());
            }
            else
            {
                std::string mimetype = g_mimeDB.getMimeType(Slice(info.nodePath));
                struct stat fileStat;
                if (stat(info.nodePath.c_str(), &fileStat) == -1)
                    throw HttpException(500);
                int fileno = open(info.nodePath.c_str(), O_RDONLY | O_CLOEXEC);
                if (fileno == -1)
                    throw HttpException(500);
                _response.initialize(200, C_SLICE("OK"), fileno, fileStat.st_size);
                _response.addHeader(C_SLICE("Content-Type"), mimetype);
                _timeout = Timeout(_response.finalizeHeader());
            }
            break;
        case NODE_TYPE_DIRECTORY:
            if (!Slice(request.queryPath).endsWith(C_SLICE("/")))
            {
                _response.initialize(301, C_SLICE("Moved Permanently"), "", 0);
                _response.addHeader(C_SLICE("Location"), Slice(request.queryPath + "/"));
                _timeout = Timeout(_response.finalizeHeader());
            }
            else if (request.method == HTTP_METHOD_POST)
            {
                std::cout << "allowUpload: " << info.getLocalRoute()->allowUpload << std::endl;
                if (info.getLocalRoute()->allowUpload)
                    uploadFile(request, info);
                else
                    throw HttpException(403);
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
                std::string autoindex = HtmlGenerator::directoryList(info.nodePath.c_str());
                _response.initialize(200, C_SLICE("OK"), autoindex);
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
            throw std::runtime_error("Method not allowed");
    }

    if (_response.getState() != HTTP_RESPONSE_FINALIZED && _process == NULL)
        throw HttpException(500);
    if (_process == NULL)
        _application._dispatcher.modify(_fileno, EPOLLOUT | EPOLLHUP, this);
}

void HttpClient::uploadFile(const HttpRequest &request, const RoutingInfo &info)
{

    uploadData data;
    data.isfinished = 0;
    printf("Uploading file\n");

    while (data.isfinished == 0)
    {
        parseupload(request, data);

        std::string path = info.nodePath + '/' + data.filename.stripStart('/').toString();

        int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (fd == -1)
            throw std::runtime_error("Unable to open file");
        int writereturn = write(fd, data.fileContent.toString().c_str(), data.fileContent.getLength());
        if (writereturn == -1)
            throw std::runtime_error("Unable to write to file");
        printf("Write return: %d\n", writereturn);
        close(fd);
        // data.isfinished = 1; //debugging only
        // printf("Mybuffer:\n%s \n", mybuffer);
    }
}

void HttpClient::parseupload(const HttpRequest &request, uploadData &data)
{
    // TODO: This needs refactoring

    printf("Parsing upload\n");

    int uploadtype = CURL;
    // creates a slice from the bodybuffer
    // if (data.fileContent)
    Slice sliceBod;
    if (data.rest.isEmpty())
    {
        sliceBod = Slice((char *)request.body.data(), request.body.size());
        // std::cout << "slicebod before first slice: " << sliceBod << std::endl;
        if (sliceBod.startsWith(C_SLICE("--")))
            sliceBod.splitStart(C_SLICE("\r\n"), data.boundary);
        else
            throw std::runtime_error("Error in Upload body");
    }
    else
        sliceBod = data.rest;
    std::cout << "Boundary: \n"
              << data.boundary << std::endl;
    std::cout << "slicebod after first slice: " << sliceBod << std::endl;
    Slice checkData;
    sliceBod.splitStart(C_SLICE(" "), checkData);
    std::cout << "Checkdata: \n"
              << checkData << std::endl;
    if (checkData == C_SLICE("Content-Disposition:"))
    {
        sliceBod.splitStart(C_SLICE(";"), data.contentDisposition);
        std::cout << "Content Dispositon: \n"
                  << data.contentDisposition << std::endl;
        std::cout << "slicebod content dispositon slice: " << sliceBod << std::endl;
        sliceBod.stripStart(' ');
        sliceBod.splitStart(C_SLICE("="), checkData);
        std::cout << "Checkdata: \n"
                  << checkData << std::endl;
        if (checkData == C_SLICE("name"))
        {
            sliceBod.splitStart(C_SLICE(";"), data.name);
            data.name.removeDoubleQuotes();
            std::cout << "Name: \n"
                      << data.name << std::endl;
            std::cout << "slicebod name slice: " << sliceBod << std::endl;
            sliceBod.stripStart(' ');
        }
        sliceBod.splitStart(C_SLICE("="), checkData);
        std::cout << "Checkdata: \n"
                  << checkData << std::endl;
        if (checkData == C_SLICE("filename"))
        {
            sliceBod.splitStart(C_SLICE("\r\n"), data.filename);
            data.filename.removeDoubleQuotes();
            std::cout << "Filename: \n"
                      << data.filename << std::endl;
            std::cout << "slicebod filename slice: " << sliceBod << std::endl;
        }
        Slice sliceTest = sliceBod;
        sliceTest.splitStart(C_SLICE(" "), checkData);
        if (checkData == C_SLICE("Content-Type:"))
        {
            uploadtype = CURL;
            sliceBod.splitStart(C_SLICE(" "), checkData);
            sliceBod.splitStart(C_SLICE("\r\n"), data.contentType);
            sliceBod.splitStart(C_SLICE("\r\n"), data.contentType);
        }
        else
        {
            uploadtype = PYTHONSCRIPT;
            sliceBod.splitStart(C_SLICE("\r\n"), data.contentType);
        }

        std::cout << "Contenttype: \n"
                  << data.contentType << std::endl;
        std::cout << "slicebod contenttype slice: \n"
                  << sliceBod << std::endl;
        // maybe add error checks here
        std::string boundary_end_string;
        if (uploadtype == PYTHONSCRIPT)
            boundary_end_string = "\r\n" + data.boundary.toString();
        else
            boundary_end_string = data.boundary.toString();

        Slice boundary_end = Slice(boundary_end_string.c_str(), data.boundary.getLength());

        if (sliceBod.splitStart(boundary_end, data.fileContent))
        {
            if (data.morethanonefile == 1)
            {
                Slice trash;
                data.fileContent.splitEndnoDel('\r', trash);
            }
            std::cout << "File Content: \n"
                      << data.fileContent << std::endl;
            std::cout << "slicebod after filecontent: \n"
                      << sliceBod << std::endl;
            if (sliceBod.startsWith(C_SLICE("--")))
            {
                data.isfinished = 1;
                std::cout << "isfinished = " << data.isfinished << std::endl;
            }
            else
            {
                sliceBod.splitStart(C_SLICE("\n"), data.rest);
                data.rest = sliceBod;
                data.morethanonefile = 1;
                std::cout << "isfinished = " << data.isfinished << std::endl;
                std::cout << "slicebod: \n"
                          << sliceBod << std::endl;
            }
            return;
        }
        else
            throw std::runtime_error("Incomplete upload body");
    }
}

/* Handles an exception that occurred in `handleEvent()` */
void HttpClient::handleException(const char *message)
{
    std::cout << "Exception while handling HTTP client event: " << message << std::endl;
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
            // TODO: This needs refactoring

            Slice iterator(reinterpret_cast<const char *>(_process->_buffer.data()), _process->_buffer.size());
            Slice header;
            iterator.splitStart(C_SLICE("\r\n\r\n"), header);

            bool initialized = false;

            Slice statusSearch = header;
            Slice temporary;
            if (statusSearch.splitStart(C_SLICE("Status:"), temporary) && (temporary.endsWith(C_SLICE("\r\n")) || temporary.isEmpty()))
            {
                Slice statusLine, statusCode, statusMessage;
                if (!statusSearch.splitStart(C_SLICE("\r\n"), statusLine))
                    statusLine = statusSearch;
                statusLine.stripStart(' ');
                if (!statusLine.splitStart(' ', statusCode))
                    throw std::runtime_error("Malformed CGI status header");
                statusMessage = statusLine;
                size_t statusCodeNumber;
                if (!Utility::parseSize(statusCode, statusCodeNumber))
                    throw std::runtime_error("aslkdjaslkdjas");
                _response.initialize(statusCodeNumber, statusMessage, &iterator[0], iterator.getLength());
            }

            if (!initialized)
                _response.initialize(200, C_SLICE("OK"), &iterator[0], iterator.getLength());

            while (header.getLength() > 0)
            {
                Slice line;
                if (!header.splitStart(C_SLICE("\r\n"), line))
                {
                    line = header;
                    header = Slice();
                }
                Slice key, value;
                if (!line.splitStart(C_SLICE(":"), key))
                    throw std::runtime_error("Malformed CGI header");
                value = line;

                if (key != C_SLICE("Status"))
                {
                    value = value.stripStart(' ');
                    _response.addHeader(key, value);
                }
            }

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
    // Find the error message
    const std::string &errorMessage = g_errorDB.getErrorType(statusCode);
    std::string errorPage = HtmlGenerator::errorPage(statusCode);

    // Build the response and set its timeout
    _response.initialize(statusCode, errorMessage, errorPage);
    _response.addHeader(C_SLICE("Content-Type"), C_SLICE("text/html"));
    _timeout = Timeout(_response.finalizeHeader());

    // Switch the dispatcher to POLLOUT
    _application._dispatcher.modify(_fileno, EPOLLOUT | EPOLLHUP, this);
}

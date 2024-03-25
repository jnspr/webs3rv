#include "http_exception.hpp"
#include "debug_utility.hpp"
#include "http_client.hpp"
#include "application.hpp"
#include "utility.hpp"
#include "mime_db.hpp"

#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <sys/stat.h>
#include <fcntl.h>

/* Constructs a HTTP client using the given socket file descriptor */
HttpClient::HttpClient(Application &application, const ServerConfig &config, int fileno, uint32_t host, uint16_t port)
    : _application(application)
    , _config(config)
    , _fileno(fileno)
    , _markedForCleanup(false)
    , _process(NULL)
    , _host(host)
    , _port(port)
    , _parser(config, host, port)
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

        Slice data(buffer, length);
        if (_parser.commit(data))
        {
            switch (_parser.getPhase())
            {
                case HTTP_REQUEST_HEADER_EXCEED:
                    printf("HTTP_REQUEST_HEADER_EXCEED\n");
                    break;
                case HTTP_REQUEST_BODY_EXCEED:
                    printf("HTTP_REQUEST_BODY_EXCEED\n");
                    break;
                case HTTP_REQUEST_MALFORMED:
                    printf("HTTP_REQUEST_MALFORMED\n");
                    break;
                case HTTP_REQUEST_COMPLETED:
                    printf("HTTP_REQUEST_COMPLETED\n");
                    Debug::printRequest(_parser.getRequest());
                    handleRequest(_parser.getRequest());
                    break;
                default:
                    break;
            }
        }
    }

    if (eventMask & EPOLLOUT)
    {
        if (_response.hasData())
            _response.transferToSocket(_fileno);
        if (!_response.hasData())
            handleException(); // mark for cleanup
    }
}

void HttpClient::handleRequest(const HttpRequest &request)
{
    if (!Utility::checkPathLevel(request.queryPath))
        throw std::runtime_error("Client tried to access above-root directory");

    RoutingInfo info = info.findRoute(_config, request.queryPath);

    if (info.status == ROUTING_STATUS_NOT_FOUND)
        throw HttpException(404);
    else if (info.status == ROUTING_STATUS_NO_ACCESS)
        throw HttpException(403);
    else if (info.status ==  ROUTING_STATUS_FOUND_LOCAL)
    {
        std::set<HttpMethod>::iterator it = info.getLocalRoute()->allowedMethods.find(request.method);
        if (it == info.getLocalRoute()->allowedMethods.end())
            throw HttpException(405);
        
        switch (info.getLocalNodeType())
        {
            case NODE_TYPE_REGULAR:
                std::cout << "NODE_TYPE_REGULAR" << std::endl;
                if (Slice(info.nodePath).endsWith(C_SLICE(".cgi"))) // 
                    _application.startCgiProcess(this, request, info);
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
                    _response.finalizeHeader();
                }
                break;
            case NODE_TYPE_DIRECTORY:
                std::cout << "NODE_TYPE_DIRECTORY" << std::endl; 
                if (request.method == HTTP_METHOD_POST)
                {
                    std::cout << "allowUpload: " <<  info.getLocalRoute()->allowUpload << std::endl;
                    if (info.getLocalRoute()->allowUpload)
                        uploadFile(request, info);
                    else
                        throw HttpException(403);
                }
                else if (Utility::queryNodeType(info.getLocalRoute()->indexFile) == 0)
                {
                    if (info.getLocalRoute()->allowListing)
                         std::cout << "Generating autoindex (Placeholder)";
                    else
                        throw HttpException(403);
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

    if (_response.getState() != HTTP_RESPONSE_FINALIZED)
        throw HttpException(500);
    _application._dispatcher.modify(_fileno, EPOLLOUT | EPOLLHUP, this);

    // TODO: Implement check if file exists on route in ServerConfig::findRoute()
    //       ^ Don't use stat() directly, use Utility::queryNodeType()

    // TODO: Find route using ServerConfig::findRoute()
    //       ^ Quit early when node type is NODE_TYPE_NO_ACCESS (HTTP 403)
    //       ^ Only 404 when the route wasn't found
    // TODO: Check allowed methods on route
    // TODO: Handle node type
    //       ^ Is directory?
    //         ^ Index file configured?
    //           ^ Quit if inaccessible (HTTP 403) or not found (HTTP 403), use it if present
    //         ^ Index file not configured?
    //           ^ Generate autoindex if enabled, quit with HTTP 403 if disabled
    //       ^ Is file?
    //         ^ Is CGI file extension? (use route's CGI map)
    //           ^ Start CGI process
    //         ^ Is regular file?
    //           ^ Get MIME type and set Content-Type
    //           ^ Send the file with Content-Length (not chunked)
}

void HttpClient::uploadFile(const HttpRequest &request, const RoutingInfo &info)
{ 

    uploadData data;
    printf("Uploading file\n");

    parseupload(request, data);

    std::string path = info.nodePath + '/' + data.filename.stripStart('/').toString();

        int fd = open(path.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (fd == -1)
            throw std::runtime_error("Unable to open file");
        int writereturn = write(fd, data.fileContent.toString().c_str(), data.fileContent.getLength());
        if (writereturn == -1)
            throw std::runtime_error("Unable to write to file");
        printf ("Write return: %d\n", writereturn);
        close(fd);
        //printf("Mybuffer:\n%s \n", mybuffer);
         

}


void HttpClient::parseupload(const HttpRequest &request, uploadData &data)
{

    printf("Parsing upload\n");
   
    int uploadtype = CURL;
    // creates a slice from the bodybuffer
     Slice sliceBod((char *) request.body.data(), request.body.size());
     //std::cout << "slicebod before first slice: " << sliceBod << std::endl;
     if (sliceBod.startsWith(C_SLICE("--")))
         sliceBod.splitStart(C_SLICE("\r\n"), data.boundary);
     else
         throw std::runtime_error("Error in Upload body");

    std::cout << "Boundary: \n" << data.boundary << std::endl;
    std::cout << "slicebod after first slice: " << sliceBod << std::endl;
    Slice checkData;
    sliceBod.splitStart(C_SLICE(" "), checkData);
    std::cout << "Checkdata: \n" << checkData << std::endl;
    if (checkData == C_SLICE("Content-Disposition:"))
    {
        sliceBod.splitStart(C_SLICE(";"), data.contentDisposition);
        std::cout << "Content Dispositon: \n" << data.contentDisposition << std::endl;
        std::cout << "slicebod content dispositon slice: " << sliceBod << std::endl;
        sliceBod.stripStart(' ');
        sliceBod.splitStart(C_SLICE("="), checkData);
        std::cout << "Checkdata: \n" << checkData << std::endl;
        if (checkData == C_SLICE("name"))
        {
            sliceBod.splitStart(C_SLICE(";"), data.name);
            data.name.removeDoubleQuotes();
            std::cout << "Name: \n" << data.name << std::endl;
            std::cout << "slicebod name slice: " << sliceBod << std::endl;
            sliceBod.stripStart(' ');
        }
        sliceBod.splitStart(C_SLICE("="), checkData);
        std::cout << "Checkdata: \n" << checkData << std::endl;
        if (checkData == C_SLICE("filename"))
        {
            sliceBod.splitStart(C_SLICE("\r\n"), data.filename);
            data.filename.removeDoubleQuotes();
            std::cout << "Filename: \n" << data.filename << std::endl;
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

        std::cout << "Contenttype: \n" << data.contentType << std::endl;
        std::cout << "slicebod contenttype slice: " << sliceBod << std::endl;
        // maybe add error checks here
        std::string boundary_end_string;
        if (uploadtype == PYTHONSCRIPT)
            boundary_end_string = "\r\n" + data.boundary.toString() + "--";
        else
            boundary_end_string = data.boundary.toString() + "--";
        
        Slice boundary_end = Slice(boundary_end_string.c_str(), data.boundary.getLength() + 2);

        if (sliceBod.splitStart(boundary_end, data.fileContent))
        {
            std::cout << "File Content: \n" << data.fileContent << std::endl;
            return;
        }
        else
            throw std::runtime_error("Incomplete upload body");
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
    if (_process->getState() == CGI_PROCESS_SUCCESS)
    {
        // start the response to client
    }
    else if (_process->getState() == CGI_PROCESS_FAILURE)
    {
        _application.closeCgiProcess(this);
    }
    else if (_process->getState() == CGI_PROCESS_TIMEOUT)
    {
        this->_markedForCleanup = true;
        this->_cleanupNext = _application._cleanupClients;
        _application._cleanupClients = this;
    }
}

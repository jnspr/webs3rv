#include "http_client.hpp"
#include "application.hpp"

#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <sys/stat.h>

/* Constructs a HTTP client using the given socket file descriptor */
HttpClient::HttpClient(Application &application, const ServerConfig &config, int fileno)
    : _application(application)
    , _config(config)
    , _fileno(fileno)
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
            handleRequest(request);
        }
    }
}

#define ALLOWED_PATH_LEVEL 1 // temporary
#define AUTOINDEX_ON 1 // placeholder for debugging
#define UPLOAD_IS_ALLOWED 1 //placeholder for debugging

void HttpClient::handleRequest(HttpRequest request)
{
       printf("Handling request\n");
    if(request.getState() != REQUEST_STATE_VALID)
        throw std::runtime_error("Http state invalid.");

    printf("Method: %d\n", request.method);

    // check if method is allowed

    if (checkpathlevel(request.getPath()) < ALLOWED_PATH_LEVEL)
        throw std::runtime_error("Access denied.");
    printf("Path: %s\n", request.getPath().c_str());
    int pathStat = checkstat(request);
printf("Pathstat: %d\n", pathStat);
    if (pathStat == ISDIR){

        if (request.method == HTTP_METHOD_POST)
        {
            if (UPLOAD_IS_ALLOWED)
            {
                uploadFile(request);
            }
            else
            {
                //403
            }
        }
        else if (AUTOINDEX_ON)
        {
            // generate directory list
            // change path do directory list file
        }
        else
            request.setPath(request.getPath() += "/${index}");
    }
    else if (pathStat == ISFILE)
    {
        if (fileextension(request.getPath()) == "cgi")
        {
            //run cgi script
        }
    }

}



void HttpClient::uploadFile(HttpRequest request)
{ 

    uploadData data;
    printf("Uploading file\n");

    parseupload(request, data);
        /*printf("file to open: %s\n", (getinputvalue(mybuffer, "filename=")).c_str());
        int fd = open((getContentLength(mybuffer, "filename=")).c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (fd == -1)
            throw std::runtime_error("Unable to open file");
        int writereturn = write(fd, mybuffer, readLength);
        if (writereturn == -1)
            throw std::runtime_error("Unable to write to file");
        printf ("Write return: %d\n", writereturn);
        close(fd);
        //printf("Mybuffer:\n%s \n", mybuffer);
         */

}


void HttpClient::parseupload(HttpRequest request, uploadData &data)
{

    printf("Parsing upload\n");
   

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
            data.name.removequotes('"');
            std::cout << "Name: \n" << data.name << std::endl;
            std::cout << "slicebod name slice: " << sliceBod << std::endl;
            sliceBod.stripStart(' ');
        }
        sliceBod.splitStart(C_SLICE("="), checkData);
        std::cout << "Checkdata: \n" << checkData << std::endl;
        if (checkData == C_SLICE("filename"))
        {
            sliceBod.splitStart(C_SLICE("\r\n"), data.filename);
            data.filename.removequotes('"');
            std::cout << "Filename: \n" << data.filename << std::endl;
            std::cout << "slicebod filename slice: " << sliceBod << std::endl;
        }
        sliceBod.splitStart(C_SLICE(" "), checkData);
        if (checkData == C_SLICE("Content-Type:"))
            sliceBod.splitStart(C_SLICE("\r\n"), data.contentType);

        std::cout << "Contenttype: \n" << data.contentType << std::endl;
        std::cout << "slicebod contenttype slice: " << sliceBod << std::endl;
        // maybe add error checks here
        std::string boundary_end_string = data.boundary.toString() + "--";
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

std::string HttpClient::fileextension(const std::string &file_or_path)
{
        size_t pos = file_or_path.find_last_of('.');
        return (file_or_path.substr(pos +1));
}


size_t HttpClient::checkpathlevel(const std::string &path)
{
    return std::count(path.begin(), path.end(), '/');
}



int HttpClient::checkstat(HttpRequest request)
{
    struct stat pathstat ={};

    int statResult = stat(request.getPath().c_str(), &pathstat);
    printf("Checking path: %s\n", request.getPath().c_str());
    printf("Stat result: %d\n", statResult);

    if (statResult != 0)
    {
        printf("Stat failed for path: %s\n", request.getPath().c_str());
        throw std::runtime_error("Path is neither an accessible file or directory");
    }
    if (S_ISDIR(pathstat.st_mode))
        return ISDIR;
    else if (S_ISREG(pathstat.st_mode))
        return ISFILE;
    return ERROR;
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

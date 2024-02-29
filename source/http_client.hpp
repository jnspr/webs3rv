#ifndef HTTP_CLIENT_hpp
#define HTTP_CLIENT_hpp

#include "config.hpp"
#include "dispatcher.hpp"
#include "http_request.hpp"

#include <time.h>

class Application;

#define ISDIR 0
#define ISFILE 1
#define ERROR 3

class HttpClient: public Sink
{
public:
    friend class Application;

    /* Constructs a HTTP client using the given socket file descriptor */
    HttpClient(Application &application, const ServerConfig &config, int fileno, time_t timeoutStart);

    /* Closes the client's file descriptor */
    ~HttpClient();

    /* Gets the client's file descriptor */
    inline int getFileno()
    {
        return _fileno;
    }
private:
    Application        &_application;
    const ServerConfig &_config;
    int                 _fileno;
    time_t              _timeoutStart;
    HttpClient         *_next;
    HttpClient         *_previous;
    HttpClient         *_cleanupNext;
    bool                _markedForCleanup;
    HttpRequest::Parser _parser;

    /* Handles one or multiple events */
    void handleEvents(uint32_t eventMask);

    /* Handles the request*/
    void handleRequest(HttpRequest request);


    /* Uploads a file sent by a POST request*/
    void uploadFile(HttpRequest request);

    /* Parses the upload body*/
    void parseupload(HttpRequest request, uploadData &data);

    /* Returns the fileextension of a given path/file */
    std::string fileextension(const std::string &file_or_path);

    /* Checks how deep a given path is*/
    size_t checkpathlevel(const std::string &path);

    /* Checks the stat of the file/directory pointed to by request PATH */
    int checkstat(HttpRequest request);

    /* Handles an exception that occurred in `handleEvent()` */
    void handleException();

    /* Disable copy-construction and copy-assignment */
    HttpClient(const HttpClient &other);
    HttpClient &operator=(const HttpClient &other);
};

/* The result of routing will be stored in here */
struct RouteResult
{
    bool                 wasFound;
    bool                 isRedirect;
    const LocalRouteConfig    *localRoute;
    const RedirectRouteConfig *redirectRoute;
    std::string          path;
};

/* Struct for parsing Uploads */
struct uploadData{
    Slice boundary;
    Slice contentDisposition;
    Slice name;
    Slice filename;
    Slice contentType;
    Slice fileContent;
    ssize_t fileSize;

    uploadData()
    {
        this->fileSize = 0;
    }
};

#endif // HTTP_CLIENT_hpp

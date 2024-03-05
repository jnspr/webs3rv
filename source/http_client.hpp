#ifndef HTTP_CLIENT_hpp
#define HTTP_CLIENT_hpp

#include "config.hpp"
#include "timeout.hpp"
#include "dispatcher.hpp"
#include "cgi_process.hpp"
#include "http_request.hpp"

#include <stdint.h>

class Application;
class CgiProcess;
struct uploadData;

#define ISDIR 0
#define ISFILE 1
#define ERROR 3



class HttpClient: public Sink
{
public:
    friend class Application;
    friend class CgiProcess;

    /* Constructs a HTTP client using the given socket file descriptor */
    HttpClient(Application &application, const ServerConfig &config, int fileno, uint32_t host, uint16_t port);

    /* Closes the client's file descriptor */
    ~HttpClient();

    /* Gets the client's file descriptor */
    inline int getFileno()
    {
        return _fileno;
    }

    /* Gets the associated timeout object */
    inline Timeout &getTimeout()
    {
        return _timeout;
    }
private:
    Application        &_application;
    const ServerConfig &_config;
    int                 _fileno;
    Timeout             _timeout;
    HttpClient         *_next;
    HttpClient         *_previous;
    HttpClient         *_cleanupNext;
    bool                _markedForCleanup;
    CgiProcess         *_process;
    uint32_t            _host;
    uint16_t            _port;

    /* Handles one or multiple events */
    void handleEvents(uint32_t eventMask);

    /* Handles the request*/
    void handleRequest(HttpRequest request);

    /* Uploads a file sent by a POST request*/
    void uploadFile(HttpRequest request);

    /* Parses the upload body*/
    void parseupload(HttpRequest request, uploadData &data);

    /* Handles an exception that occurred in `handleEvent()` */
    void handleException();

    /* Handles a CGI process event */
    void handleCgiState();

    /* Disable copy-construction and copy-assignment */
    HttpClient(const HttpClient &other);
    HttpClient &operator=(const HttpClient &other);
};

/* Struct for parsing Uploads */
struct uploadData
{
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

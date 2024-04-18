#ifndef HTTP_CLIENT_hpp
#define HTTP_CLIENT_hpp

#include "config.hpp"
#include "timeout.hpp"
#include "dispatcher.hpp"
#include "cgi_process.hpp"
#include "http_request.hpp"
#include "routing.hpp"
#include "http_response.hpp"
#include "http_request_parser.hpp"

#include <stdint.h>

class Application;
class CgiProcess;

class HttpClient: public Sink
{
public:
    friend class Application;
    friend class CgiProcess;

    /* Constructs a HTTP client using the given socket file descriptor */
    HttpClient(Application &application, const ServerConfig *config, int fileno, uint32_t host, uint16_t port);

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
    const ServerConfig *_config;
    int                 _fileno;
    Timeout             _timeout;
    HttpClient         *_next;
    HttpClient         *_previous;
    HttpClient         *_cleanupNext;
    bool                _waitingForClose;
    bool                _markedForCleanup;
    CgiProcess         *_process;
    uint32_t            _host;
    uint16_t            _port;
    HttpRequestParser   _parser;
    HttpResponse        _response;

    /* Handles one or multiple events */
    void handleEvents(uint32_t eventMask);

    /* Handles the request*/
    void handleRequest(const HttpRequest &request); // take reference for all the requests

    /* Initializes the response object to use a static file */
    void setupFileResponse(size_t statusCode, Slice statusMessage, const std::string &path);

    /* Handles an exception that occurred in `handleEvent()` */
    void handleException(const char *message);

    /* Handles a CGI process event */
    void handleCgiState();

    /* Marks the client to be cleaned up during the next cleanup cycle */
    void markForCleanup();

    /* Disable copy-construction and copy-assignment */
    HttpClient(const HttpClient &other);
    HttpClient &operator=(const HttpClient &other);

    /* Create error response */
    void createErrorResponse(size_t statusCode);
};

#endif // HTTP_CLIENT_hpp

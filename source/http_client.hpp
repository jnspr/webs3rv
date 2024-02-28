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

class HttpClient: public Sink
{
public:
    friend class Application;
    friend class CgiProcess;

    /* Constructs a HTTP client using the given socket file descriptor */
    HttpClient(Application &application, const ServerConfig &config, int fileno);

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
    HttpRequest::Parser _parser;
    CgiProcess         *_process;

    /* Handles one or multiple events */
    void handleEvents(uint32_t eventMask);

    /* Handles an exception that occurred in `handleEvent()` */
    void handleException();

    /* Handles a CGI process event */
    void handleCgiState();

    /* Disable copy-construction and copy-assignment */
    HttpClient(const HttpClient &other);
    HttpClient &operator=(const HttpClient &other);
};

#endif // HTTP_CLIENT_hpp

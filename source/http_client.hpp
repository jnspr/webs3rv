#ifndef HTTP_CLIENT_hpp
#define HTTP_CLIENT_hpp

#include "config.hpp"
#include "dispatcher.hpp"
#include "http_request.hpp"

#include <time.h>

class Application;

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

    /* Handles an exception that occurred in `handleEvent()` */
    void handleException();

    /* Disable copy-construction and copy-assignment */
    HttpClient(const HttpClient &other);
    HttpClient &operator=(const HttpClient &other);
};

#endif // HTTP_CLIENT_hpp

#ifndef HTTP_SERVER_hpp
#define HTTP_SERVER_hpp

#include "config.hpp"
#include "dispatcher.hpp"

class Application;

class HttpServer: public Sink
{
public:
    /* Constructs an HTTP server according to its configuration */
    HttpServer(Application &application, const ServerConfig &config);

    /* Closes the server's listening socket */
    ~HttpServer();

    /* Gets the file descriptor of the server's listening socket */
    inline int getFileno()
    {
        return _fileno;
    }
private:
    Application        &_application;
    const ServerConfig &_config;
    int                 _fileno;

    /* Handles one or multiple events */
    void handleEvents(uint32_t eventMask);

    /* Handles an exception that occurred in `handleEvent()` */
    void handleException();

    /* Disable copy-construction and copy-assignment */
    HttpServer(const HttpServer &other);
    HttpServer &operator=(const HttpServer &other);
};

#endif // HTTP_SERVER_hpp

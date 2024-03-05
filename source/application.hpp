#ifndef APPLICATION_hpp
#define APPLICATION_hpp

#include "config.hpp"
#include "dispatcher.hpp"
#include "http_server.hpp"
#include "http_client.hpp"

#include <vector>

class Application
{
public:
    friend class HttpServer;
    friend class HttpClient;

    /* Constructs the main application object */
    Application(const ApplicationConfig &config);

    /* Releases all application resources */
    ~Application();

    /* Sets up the server according to the constructor-supplied configuration */
    void configure();

    /* Enters the application's main loop until an exit condition occurs */
    void mainLoop();
private:
    const ApplicationConfig   &_config;
    Dispatcher                 _dispatcher;
    std::vector<HttpServer *>  _servers;
    HttpClient                *_clients;
    HttpClient                *_cleanupClients;
    bool                       _wasConfigured;

    /* Starts to manage the given client file descriptor according to its server's config */
    void takeClient(int fileno, const ServerConfig &config, uint32_t host, uint16_t port);

    /* Immediately releases and destroys the given client; DO NOT use from outside of this class */
    void removeClient(HttpClient *client);

    /* Starts CGI processes for the given client */
    void startCgiProcess(HttpClient *client, const HttpRequest &request, const RouteResult &routeResult);

    /*Close CGI processes for the given client */
    void closeCgiProcess(HttpClient *client);
};

#endif // APPLICATION_hpp

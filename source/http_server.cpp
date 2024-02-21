#include "application.hpp"
#include "http_server.hpp"

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/* Constructs an HTTP server according to its configuration */
HttpServer::HttpServer(Application &application, const ServerConfig &config)
    : _application(application)
    , _config(config)
{
    if ((_fileno = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        throw std::runtime_error("Unable to create TCP listener socket");

    // Populate the binding address
    sockaddr_in address = {};
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = htonl(config.host);
    address.sin_port        = htons(config.port);

    // Prevent the OS from holding the TCP port when the server exits
    int option = 1;
    setsockopt(_fileno, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // Bind to the binding address and start listening
    if (bind(_fileno, (const sockaddr *)&address, sizeof(address)) != 0)
    {
        close(_fileno);
        throw std::runtime_error("Unable to bind TCP listener socket");
    }
    if (listen(_fileno, 128) != 0)
    {
        close(_fileno);
        throw std::runtime_error("Unable to listen on TCP listener socket");
    }
}

/* Closes the server's listening socket */
HttpServer::~HttpServer()
{
    close(_fileno);
}

/* Handles one or multiple events */
void HttpServer::handleEvents(uint32_t eventMask)
{
    if ((eventMask & EPOLLIN) == 0)
        return;

    int fileno;
    if ((fileno = accept(_fileno, NULL, NULL)) < 0)
        throw std::runtime_error("Unable to accept client");

    try
    {
        _application.takeClient(fileno, _config);
    }
    catch (...)
    {
        close(fileno);
        throw;
    }
}

/* Handles an exception that occurred in `handleEvent()` */
void HttpServer::handleException()
{
}

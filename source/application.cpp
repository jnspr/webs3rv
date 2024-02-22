#include "application.hpp"

/* Constructs the main application object */
Application::Application(const ApplicationConfig &config)
    : _config(config)
    , _dispatcher(128)
    , _clients(NULL)
    , _cleanupClients(NULL)
    , _wasConfigured(false)
{
    // Check if the configuration is valid
    if (config.servers.size() == 0)
        throw std::runtime_error("Configuration has no servers");
}

/* Sets up the server according to the constructor-supplied configuration */
void Application::configure()
{
    if (_wasConfigured)
        throw std::logic_error("Invalid usage; .configure() must only be called once");

    // Create a HTTP server for each configuration entry
    for (size_t index = 0; index < _config.servers.size(); index++)
    {
        const ServerConfig &serverConfig = _config.servers[index];
        HttpServer *server = new HttpServer(*this, serverConfig);
        try
        {
            _servers.push_back(server);
        }
        catch (...)
        {
            delete server;
        }
        _dispatcher.subscribe(server->getFileno(), EPOLLIN, server);
    }

    _wasConfigured = true;
}

/* Releases all application resources */
Application::~Application()
{
    // All sinks can be destroyed immediately because the dispatcher is not going to be used again

    // Destroy clients
    HttpClient *nextClient, *headClient = _clients;
    while (headClient != NULL)
    {
        nextClient = headClient->_next;
        delete headClient;
        headClient = nextClient;
    }

    // Destroy servers
    for (size_t index = 0; index < _servers.size(); index++)
        delete _servers[index];
}

/* Enters the application's main loop until an exit condition occurs */
void Application::mainLoop()
{
    if (!_wasConfigured)
        throw std::logic_error("Invalid usage; .configure() must be called before .mainLoop()");

    HttpClient *headClient, *nextClient;
    for (;;)
    {
        // Wait up to 5000 to dispatch any event(s)
        _dispatcher.dispatch(5000);

        // TODO: Implement timeouts (mark for cleanup)

        // Remove all clients that were marked for cleanup
        headClient = _cleanupClients;
        _cleanupClients = NULL;
        while (headClient != NULL)
        {
            nextClient = headClient->_next;
            removeClient(headClient);
            headClient = nextClient;
        }
    }
}

/* Starts to manage the given client file descriptor according to its server's config */
void Application::takeClient(int fileno, const ServerConfig &config)
{
    // Wrap the client into an object
    (void)config;
    HttpClient *client = new HttpClient(*this, config, fileno, 0);

    // Subscribe client sink to read events
    try
    {
        _dispatcher.subscribe(client->getFileno(), EPOLLIN | EPOLLHUP, client);
    }
    catch (...)
    {
        delete client;
        throw;
    }

    // Link the client object
    client->_previous = NULL;
    client->_next = _clients;
    _clients = client;
}

/* Immediately releases and destroys the given client; DO NOT use from outside of this class */
void Application::removeClient(HttpClient *client)
{
    // Unlink root node
    if (_clients == client)
        _clients = client->_next;

    // Unlink neighboring nodes
    if (client->_previous != NULL)
        client->_previous->_next = client->_next;
    if (client->_next != NULL)
        client->_next->_previous = client->_previous;

    // Unsubscribe and destroy the client
    _dispatcher.unsubscribe(client->getFileno());
    delete client;
}

#ifndef CONFIG_hpp
#define CONFIG_hpp

#include "http_constants.hpp"
#include "config_tokenizer.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <stdint.h>

struct ServerConfig;

/* Configuration for a route that requires further processing by the server */
struct LocalRouteConfig
{
    std::string                        path;
    std::set<HttpMethod>               allowedMethods;
    std::string                        rootDirectory;
    std::string                        uploadDirectory;
    std::string                        indexFile;
    bool                               allowUpload;
    bool                               allowListing;
    std::map<std::string, std::string> cgiTypes;
    std::set<TokenKind>                parsedTokens;

    LocalRouteConfig();
};

/* Configuration for a route that immediately redirects the client */
struct RedirectRouteConfig
{
    std::string          path;
    std::set<HttpMethod> allowedMethods;
    std::string          redirectLocation;
};

/* Virtual server configuration */
struct ServerConfig
{
    std::vector<std::string>         name;
    uint32_t                         host;
    uint16_t                         port;
    std::map<int, std::string>       errorPages;
    size_t                           maxBodySize;
    std::vector<LocalRouteConfig>    localRoutes;
    std::vector<RedirectRouteConfig> redirectRoutes;
    std::set<TokenKind>              parsedTokens;
    ServerConfig                    *nextEndpoint;

    ServerConfig();

    /* Searches for the right server configuration based on the name, returns `this` if not found */
    const ServerConfig *findServer(Slice name) const;
};

/* Global application configuration; can contain many virtual servers */
struct ApplicationConfig
{
    std::vector<ServerConfig> servers;
};

#endif // CONFIG_hpp

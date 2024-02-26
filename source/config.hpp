#ifndef CONFIG_hpp
#define CONFIG_hpp

#include "http_constants.hpp"
#include "ConfigTokenizer.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <stdint.h>

/* Forward declaration*/
struct RouteResult;


/* Configuration for a route that requires further processing by the server */
struct LocalRouteConfig
{
    std::string path;
    std::set<HttpMethod> allowedMethods;
    std::string rootDirectory;
    std::string uploadDirectory;
    std::string indexFile;
    bool allowUploads;
    bool allowListing;
    std::map<std::string, std::string> cgiTypes;

    std::set<TokenKind> parsedTokens;
};

/* Configuration for a route that immediately redirects the client */
struct RedirectRouteConfig
{
    std::string path;
    std::set<HttpMethod> allowedMethods;
    std::string redirectLocation;
};

/* Virtual server configuration */
struct ServerConfig
{
    std::string                        name;
    uint32_t                           host;
    uint16_t                           port;
    std::map<int, std::string>         errorPages;
    size_t                             maxBodySize;
    std::vector<LocalRouteConfig>      localRoutes;
    std::vector<RedirectRouteConfig>   redirectRoutes;
    std::set<TokenKind> parsedTokens;

    RouteResult findRoute(Slice path) const;
};

/* Global application configuration; can contain many virtual servers */
struct ApplicationConfig
{
    std::vector<ServerConfig> servers;
};

/* Prints the given configuration to standard output */
void printConfig(const ApplicationConfig &config);

#endif // CONFIG_hpp

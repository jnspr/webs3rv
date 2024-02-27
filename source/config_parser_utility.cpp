#include "config_parser_utility.hpp"

#include <stdlib.h>

// Checks if the token is already defined for all tokens exept KW_ERROR_PAGE and KW_LOCATION
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind)
{
    if (tokenKind != KW_ERROR_PAGE && tokenKind != KW_LOCATION)
    {
        if (serverConfig.parsedTokens.find(tokenKind) != serverConfig.parsedTokens.end())
            throw ConfigException("Error: Redundant token", offset);
        serverConfig.parsedTokens.insert(tokenKind);
    }
}
// Checks if the error redirect (error page) is already defined
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind,
                      std::map<int, std::string> &currentErrorRedirect)
{
    if (serverConfig.errorPages.find(currentErrorRedirect.begin()->first) !=
        serverConfig.errorPages.end())
        throw ConfigException("Error: Redundant token error redirect", offset);
    serverConfig.parsedTokens.insert(tokenKind);
}
// Checks if the route path (location) is already defined
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind,
                      std::string &currentRoutePath)
{
    if (serverConfig.parsedTokens.size() != 0)
    {
        for (std::vector<LocalRouteConfig>::iterator localRouteConfigIt =
                 serverConfig.localRoutes.begin();
             localRouteConfigIt != serverConfig.localRoutes.end(); ++localRouteConfigIt)

        {
            if (localRouteConfigIt->path == currentRoutePath)
                throw ConfigException("Error: Redundant token route path", offset);
        }
    }
    serverConfig.parsedTokens.insert(tokenKind);
}
// Checks if the token is already defined in the current local route
void isRedundantToken(size_t offset, LocalRouteConfig &localRouteConfig, TokenKind tokenKind)
{
    if (localRouteConfig.parsedTokens.find(tokenKind) != localRouteConfig.parsedTokens.end())
        throw ConfigException("Error: Redundant or contradictory token", offset);
    localRouteConfig.parsedTokens.insert(tokenKind);
}

// Checks if the cgi file extension is already defined
void isRedundantToken(size_t offset, LocalRouteConfig &localRouteConfig, TokenKind tokenKind,
                      std::map<std::string, std::string> &currentCgiFileExtension)
{
    if (localRouteConfig.cgiTypes.find(currentCgiFileExtension.begin()->first) !=
        localRouteConfig.cgiTypes.end())
        throw ConfigException("Error: Redundant token CGI file extension", offset);
    localRouteConfig.parsedTokens.insert(tokenKind);
}

// Check if required config entries/tokens are missing
void isServerTokensMissing(std::set<TokenKind> &parsedTokens, size_t offset)
{
    if (parsedTokens.find(KW_PORT) == parsedTokens.end() ||
        parsedTokens.find(KW_LOCATION) == parsedTokens.end() ||
        parsedTokens.find(KW_SERVER_NAME) == parsedTokens.end())
        throw ConfigException("Error: Missing server config required token", offset);
}

// Check if required route/location config entries/tokens are missing
void isRouteTokensMissing(std::set<TokenKind> &parsedTokens, size_t offset)
{
    if ((parsedTokens.find(KW_ROOT) == parsedTokens.end() &&
         parsedTokens.find(KW_REDIRECT_ADDRESS) == parsedTokens.end()) ||
        parsedTokens.find(KW_CGI) == parsedTokens.end())
        throw ConfigException("Error: Missing route/location required token", offset);
}

void checkValidCgiFileExtension(const std::string &cgiFileExtension, size_t offset)
{
    if (cgiFileExtension != ".php" && cgiFileExtension != ".py" && cgiFileExtension != ".cgi")
        throw ConfigException("Error: Invalid CGI file extension", offset);
}

std::string readFile(const char *path)
{
    std::ifstream inputStream(path);
    if (!inputStream.is_open())
        throw std::runtime_error("Unable to open file: " + std::string(path));

    std::stringstream stringStream;
    stringStream << inputStream.rdbuf();
    if (!inputStream.good())
        throw std::runtime_error("Unable to read file: " + std::string(path));
    return stringStream.str();
}

// Checks if the string is a valid IP address consisting of 4 segments and returns the IP address as
// a uint32_t
uint32_t ConvertIpString(std::string ip)
{
    uint32_t ip_num = 0;
    std::istringstream iss(ip);
    std::string segment;
    int segmentsCount = 0;

    while (std::getline(iss, segment, '.'))
    {
        if (!isValidIpSegment(segment) || segmentsCount >= 4)
        {
            throw std::invalid_argument("Invalid IP address format");
        }
        ip_num |= (static_cast<uint32_t>(std::atoi(segment.c_str())) << ((3 - segmentsCount) * 8));
        segmentsCount++;
    }

    if (segmentsCount != 4)
    {
        throw std::invalid_argument("Invalid IP address format");
    }

    return ip_num;
}

// Checks if the string is a valid IP segment (0-255)
bool isValidIpSegment(const std::string &ipSegment)
{
    if (ipSegment.empty() || ipSegment.size() > 3)
        return false;
    for (std::string::const_iterator it = ipSegment.begin(); it != ipSegment.end(); ++it)
    {
        if (!isdigit(*it))
            return false;
    }

    int num = std::atoi(ipSegment.c_str());
    if (num < 0 || num > 255)
        return false;
    return true;
}

// Print config
void printConfigP(const ApplicationConfig &config)
{
    std::cout << "\033[34m\n\n\n"
              << "Print Configs\n"
              << "\033[0m";
    for (std::vector<ServerConfig>::const_iterator serverIt = config.servers.begin();
         serverIt != config.servers.end(); ++serverIt)
    {
        std::cout << "\033[33m\n"
                  << "Virtual Server:\n"
                  << "\033[0m";
        std::cout << "Server name: " << serverIt->name << std::endl;
        std::cout << "Server host ip: " << serverIt->host << std::endl;
        std::cout << "Server host ip as doted decimal: " << (serverIt->host >> 24 & 255) << "."
                  << (serverIt->host >> 16 & 255) << "." << (serverIt->host >> 8 & 255) << "."
                  << (serverIt->host & 255) << std::endl;
        std::cout << "Server port: " << serverIt->port << std::endl;
        std::cout << "Server max body size: " << serverIt->maxBodySize << std::endl;
        std::cout << "Server error redirects: ";
        for (std::map<int, std::string>::const_iterator it = serverIt->errorPages.begin();
             it != serverIt->errorPages.end(); ++it)
            std::cout << it->first << " => " << it->second << '\n';
        std::cout << std::endl;

        for (std::vector<LocalRouteConfig>::const_iterator localRouteIt =
                 serverIt->localRoutes.begin();
             localRouteIt != serverIt->localRoutes.end(); ++localRouteIt)
        {
            std::cout << "\033[32m\n"
                      << "Local Route:\n"
                      << "\033[0m";
            std::cout << "Local route path: " << localRouteIt->path << std::endl;
            std::cout << "Local route root directory: " << localRouteIt->rootDirectory << std::endl;
            std::cout << "Local route allowed methods: ";
            for (std::set<HttpMethod>::const_iterator it = localRouteIt->allowedMethods.begin();
                 it != localRouteIt->allowedMethods.end(); ++it)
                std::cout << *it << " ";
            std::cout << std::endl;
            std::cout << "Local route allow directory listing: "
                      << localRouteIt->allowListing << std::endl;
            std::cout << "Local route index file: " << localRouteIt->indexFile << std::endl;
            std::cout << "Local route CGI file extensions: ";
            for (std::map<std::string, std::string>::const_iterator it =
                     localRouteIt->cgiTypes.begin();
                 it != localRouteIt->cgiTypes.end(); ++it)
                std::cout << it->first << " => " << it->second << '\n';
            // for (std::set<std::string>::const_iterator it =
            // localRouteIt->cgiTypes.begin();
            //      it != localRouteIt->cgiTypes.end(); ++it)
            //     std::cout << *it << " ";
            std::cout << std::endl;
            std::cout << "Local route allow file upload: " << localRouteIt->allowUploads
                      << std::endl;
            std::cout << "Local route upload path: " << localRouteIt->uploadDirectory << std::endl;
        }

        for (std::vector<RedirectRouteConfig>::const_iterator redirectRouteIt =
                 serverIt->redirectRoutes.begin();
             redirectRouteIt != serverIt->redirectRoutes.end(); ++redirectRouteIt)
        {
            std::cout << "Redirect route url redirection: " << redirectRouteIt->redirectLocation
                      << std::endl;
            std::cout << "Redirect route path: " << redirectRouteIt->path << std::endl;
            std::cout << "Redirect route allowed methods: ";
            for (std::set<HttpMethod>::const_iterator it = redirectRouteIt->allowedMethods.begin();
                 it != redirectRouteIt->allowedMethods.end(); ++it)
                std::cout << *it << " ";
            std::cout << std::endl;
        }
    }
    std::cout << "Finished printing config\n\n\n";
}
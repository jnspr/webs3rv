#include "config_parser_utility.hpp"

#include <stdlib.h>

// Checks if the token is already defined for all tokens exept KW_ERROR_PAGE and KW_LOCATION
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind, std::string config_input)
{
    if (tokenKind != KW_ERROR_PAGE && tokenKind != KW_LOCATION)
    {
        if (serverConfig.parsedTokens.find(tokenKind) != serverConfig.parsedTokens.end())
            throw ConfigException("Error: Redundant token", config_input, offset);
        serverConfig.parsedTokens.insert(tokenKind);
    }
}
// Checks if the error redirect (error page) is already defined
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind,
                      std::map<int, std::string> &currentErrorRedirect, std::string config_input)
{
    if (serverConfig.errorPages.find(currentErrorRedirect.begin()->first) !=
        serverConfig.errorPages.end())
        throw ConfigException("Error: Redundant token error redirect", config_input, offset);
    serverConfig.parsedTokens.insert(tokenKind);
}
// Checks if the route path (location) is already defined
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind,
                      std::string &currentRoutePath, std::string config_input)
{
    if (serverConfig.parsedTokens.size() != 0)
    {
        for (std::vector<LocalRouteConfig>::iterator localRouteConfigIt =
                 serverConfig.localRoutes.begin();
             localRouteConfigIt != serverConfig.localRoutes.end(); ++localRouteConfigIt)

        {
            if (localRouteConfigIt->path == currentRoutePath)
                throw ConfigException("Error: Redundant token route path", config_input, offset);
        }
    }
    serverConfig.parsedTokens.insert(tokenKind);
}
// Checks if the token is already defined in the current local route
void isRedundantToken(size_t offset, LocalRouteConfig &localRouteConfig, TokenKind tokenKind, std::string config_input)
{
    if (localRouteConfig.parsedTokens.find(tokenKind) != localRouteConfig.parsedTokens.end())
        throw ConfigException("Error: Redundant or contradictory token", config_input, offset);
    localRouteConfig.parsedTokens.insert(tokenKind);
}

// Checks if the cgi file extension is already defined
void isRedundantToken(size_t offset, LocalRouteConfig &localRouteConfig, TokenKind tokenKind,
                      std::map<std::string, std::string> &currentCgiFileExtension, std::string config_input)
{
    if (localRouteConfig.cgiTypes.find(currentCgiFileExtension.begin()->first) !=
        localRouteConfig.cgiTypes.end())
        throw ConfigException("Error: Redundant token CGI file extension", config_input, offset);
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

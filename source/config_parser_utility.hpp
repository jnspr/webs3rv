#ifndef CONFIG_PARSER_UTILITY_hpp
#define CONFIG_PARSER_UTILITY_hpp

#include "config.hpp"
#include "config_parser.hpp"
#include "config_tokenizer.hpp"

// Checks if the token is already defined for all tokens exept KW_ERROR_PAGE and KW_LOCATION
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind, std::string config_input);
// Checks if the error redirect (error page) is already defined
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind,
                      std::map<int, std::string> &currentErrorRedirect, std::string config_input);
// Checks if the route path (location) is already defined
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind,
                      std::string &currentRoutePath, std::string config_input);
// Checks if the token is already defined in the current local route
void isRedundantToken(size_t offset, LocalRouteConfig &localRouteConfig, TokenKind tokenKind, std::string config_input);
// Checks if the cgi file extension is already defined
void isRedundantToken(size_t offset, LocalRouteConfig &localRouteConfig, TokenKind tokenKind,
                      std::map<std::string, std::string> &currentCgiFileExtension, std::string config_input);

// Check if required server config entries/tokens are missing
void isServerTokensMissing(std::set<TokenKind> &parsedTokens, size_t offset);
// Check if required route/location config entries/tokens are missing
void isRouteTokensMissing(std::set<TokenKind> &parsedTokens, size_t offset);
// Check if it is a valid CGI file extension
void checkValidCgiFileExtension(const std::string &cgiFileExtension, size_t offset);

// Read then all content of the config file
std::string readFile(const char *path);

// Checks if the string is a valid IP address consisting of 4 segments and returns the IP address as
// a uint32_t
uint32_t ConvertIpString(std::string ip);
// Checks if the string is a valid IP segment (0-255)
bool isValidIpSegment(const std::string &ipSegment);

#endif // CONFIG_PARSER_UTILITY_hpp

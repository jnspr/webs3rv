/* ************************************************************************** */
/*                                                                            */
/*  Unbreakable Webserv                               _             _         */
/*                                                   / /\      _   / /\       */
/*                                                  / / /    / /\ / /  \      */
/*                                                 / / /    / / // / /\ \__   */
/*                                                / / /_   / / // / /\ \___\  */
/*                                               / /_//_/\/ / / \ \ \ \/___/  */
/*                                              / _______/\/ /   \ \ \        */
/*                                             / /  \____\  /_    \ \ \       */
/*   By: cgodecke (Dsite42 on Github)         /_/ /\ \ /\ \//_/\__/ / /       */
/*       jsprenge (jnspr on GitHub)           \_\//_/ /_/ / \ \/___/ /        */
/*       fvon-nag (flx25 on GitHub)               \_\/\_\/   \_____\/         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSERUTILITY_HPP
#define CONFIGPARSERUTILITY_HPP

#include "ConfigParser.hpp"
#include "ConfigTokenizer.hpp"
#include "config.hpp"

// Checks if the token is already defined for all tokens exept KW_ERROR_PAGE and KW_LOCATION
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind);
// Checks if the error redirect (error page) is already defined
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind,
                      std::map<int, std::string> &currentErrorRedirect);
// Checks if the route path (location) is already defined
void isRedundantToken(size_t offset, ServerConfig &serverConfig, TokenKind tokenKind,
                      std::string &currentRoutePath);
// Checks if the token is already defined in the current local route
void isRedundantToken(size_t offset, LocalRouteConfig &localRouteConfig, TokenKind tokenKind);
// Checks if the cgi file extension is already defined
void isRedundantToken(size_t offset, LocalRouteConfig &localRouteConfig, TokenKind tokenKind,
                      std::map<std::string, std::string> &currentCgiFileExtension);

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

// Print config
void printConfigP(const ApplicationConfig &config);

#endif
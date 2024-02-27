#ifndef CONFIG_PARSER_hpp
#define CONFIG_PARSER_hpp

#include "config.hpp"
#include "http_constants.hpp"
#include "config_tokenizer.hpp"
#include "config_parser_utility.hpp"

#include <sstream>
#include <stdint.h>

class ConfigParser
{
private:
    std::vector<Token> _tokens;
    size_t _current;
    std::string _config_input;

    // Parsing ServerConfig
    ServerConfig parseServerConfig();
    void parsePortOrIp(ServerConfig &serverConfig);

    // Parsing LocalRouteConfig
    LocalRouteConfig parseLocalRouteConfig(ServerConfig &serverConfig);
    std::set<HttpMethod> parseAllowedHttpMethods(ServerConfig &serverConfig,
                                                 LocalRouteConfig &localRouteConfig);
    bool parseDirectoryListing();
    std::map<std::string, std::string> parseCgiFileExtensions();

    // Parsing RedirectRouteConfig
    RedirectRouteConfig parseRedirectRouteConfig(LocalRouteConfig &localRouteConfig);

    // Token handling
    void expect(TokenKind kind);
    void moveToNextToken();
    Token &currentToken();

    // Parsing data types
    std::string parseString();
    uint16_t parseUint16();
    size_t parseSizeT();
    std::map<int, std::string> parseErrorRedirects();

public:
    // Inner class for exceptions specific to ConfigParser
    struct ParserException : public std::exception
    {
        mutable std::string formattedMessage;
        size_t offset;
        std::string config_input;

        ParserException(const std::string &formattedMessage, size_t offset, const std::string &config_input);
        ParserException(const std::string &formattedMessage, size_t offset);

        ~ParserException() throw();
        virtual const char *what() const throw();

        size_t getOffset() const;
        std::string getConfigInput() const;
    };

    explicit ConfigParser(const std::vector<Token> &_tokens);
    ApplicationConfig parse();

    static ApplicationConfig createConfig(const char *configPath);
};

#endif // CONFIG_PARSER_hpp

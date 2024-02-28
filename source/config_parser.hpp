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
    explicit ConfigParser(const std::vector<Token> &_tokens);
    ApplicationConfig parse();

    static ApplicationConfig createConfig(const char *configPath);
};

class ConfigException: public std::exception
{
public:
    /* Constructs a parser exception using the given reason, source and offset */
    ConfigException(const std::string &reason, const std::string &source, size_t offset);

    /* Constructs a parser exception using the given reason and offset */
    ConfigException(const std::string &reason, size_t offset);

    /* Constructs a parser exception using the given reason */
    ConfigException(const std::string &reason);

    /* Empty method; declares the destructor as non-throwing */
    ~ConfigException() throw();

    /* Gets the cause of the error as a C-style string */
    const char *what() const throw();

    /* Gets the cause of the error */
    inline const std::string &getReason() const
    {
        return _reason;
    }

    /* Gets the offending source string */
    inline const std::string &getSource() const
    {
        return _source;
    }

    /* Gets the offending offset */
    inline size_t getOffset() const
    {
        return _offset;
    }
private:
    std::string _reason;
    std::string _source;
    size_t      _offset;
};

#endif // CONFIG_PARSER_hpp

#include "config_parser.hpp"

// Constructor
ConfigParser::ConfigParser(const std::vector<Token> &tokens) : _tokens(tokens), _current(0)
{
}

// Methods
ApplicationConfig ConfigParser::parse()
{
    ApplicationConfig config;
    while (_current < _tokens.size())
    {
        if (_tokens[_current].kind == KW_SERVER)
            config.servers.push_back(parseServerConfig());
        else
            throw ConfigException("Error: Unexpected token in config", _config_input, _tokens[_current].offset);
    }
    return config;
}

// Parsing ServerConfig
ServerConfig ConfigParser::parseServerConfig()
{
    ServerConfig serverConfig;
    std::map<int, std::string> currentErrorRedirect;

    moveToNextToken();
    expect(SY_BRACE_OPEN);
    while (currentToken().kind != SY_BRACE_CLOSE)
    {
        switch (currentToken().kind)
        {
        case SY_COMMEND:
            moveToNextToken();
            break;
        case KW_SERVER_NAME:
            isRedundantToken(_tokens[_current].offset, serverConfig, KW_SERVER_NAME, _config_input);
            moveToNextToken();
            serverConfig.name = parseServerNames();
            expect(SY_SEMICOLON);
            break;
        case KW_PORT:
            isRedundantToken(_tokens[_current].offset, serverConfig, KW_PORT, _config_input);
            moveToNextToken();
            parsePortOrIp(serverConfig);
            expect(SY_SEMICOLON);
            break;
        case KW_MAX_BODY_SIZE:
            isRedundantToken(_tokens[_current].offset, serverConfig, KW_MAX_BODY_SIZE, _config_input);
            moveToNextToken();
            serverConfig.maxBodySize = parseSizeT();
            expect(SY_SEMICOLON);
            break;
        case KW_ERROR_PAGE:
            moveToNextToken();
            currentErrorRedirect = parseErrorRedirects();
            isRedundantToken(_tokens[_current - 2].offset, serverConfig, KW_ERROR_PAGE,
                             currentErrorRedirect, _config_input);
            serverConfig.errorPages.insert(currentErrorRedirect.begin(),
                                           currentErrorRedirect.end());
            expect(SY_SEMICOLON);
            break;
        case KW_LOCATION:
            serverConfig.localRoutes.push_back(parseLocalRouteConfig(serverConfig));
            serverConfig.parsedTokens.insert(KW_LOCATION);
            break;
        default:
            throw ConfigException("Error: Unexpected token in server config",
                                  _config_input, _tokens[_current].offset);
        }
    }

    expect(SY_BRACE_CLOSE);
    isServerTokensMissing(serverConfig.parsedTokens, _tokens[_current - 1].offset, _config_input);
    return serverConfig;
}

void ConfigParser::parsePortOrIp(ServerConfig &serverConfig)
{
    expect(DATA);
    if (currentToken().data.find(':') == std::string::npos)
        serverConfig.port = parseUint16();
    else
    {
        std::string ip = currentToken().data.substr(0, currentToken().data.find(':'));
        serverConfig.host = ConvertIpString(ip, _tokens[_current].offset, _config_input);
        std::string port = currentToken().data.substr(currentToken().data.find(':') + 1);
        std::istringstream iss(port);
        iss >> serverConfig.port;
        if (iss.fail() || !iss.eof())
            throw ConfigException("Error: Invalid port number", _config_input, _tokens[_current].offset);
        moveToNextToken();
    }
}

// Token handling
void ConfigParser::expect(TokenKind kind)
{
    if (currentToken().kind != kind)
        throw ConfigException("Error: Unexpected token", _config_input, _tokens[_current].offset);
    if (currentToken().kind != DATA)
        moveToNextToken();
}

Token &ConfigParser::currentToken()
{
    return _tokens[_current];
}

void ConfigParser::moveToNextToken()
{
    if (_current < _tokens.size())
        ++_current;
    else
        throw ConfigException("Error: Unexpected end of tokens", _config_input, _tokens[_current].offset);
}

// Parsing data types
std::string ConfigParser::parseString()
{
    expect(DATA);
    std::string str = currentToken().data;
    moveToNextToken();
    return str;
}

std::vector<std::string> ConfigParser::parseServerNames()
{
    std::vector<std::string> serverNames;
    while (currentToken().kind != SY_SEMICOLON)
    {
        expect(DATA);
        serverNames.push_back(currentToken().data);
        moveToNextToken();
    }
    return serverNames;
}

std::string ConfigParser::parseLocalRoutePath()
{
    expect(DATA);
    std::string str = currentToken().data;
    if (str.empty() || str[0] != '/')
        throw ConfigException("Error: Invalid path. Shall begin with /", _config_input, _tokens[_current].offset);
    moveToNextToken();
    return str;
}

uint16_t ConfigParser::parseUint16()
{
    expect(DATA);
    std::istringstream iss(currentToken().data);
    uint16_t num;
    iss >> num;
    if (iss.fail() || !iss.eof())
        throw ConfigException("Error: Invalid uint16_t value", _config_input, _tokens[_current].offset);
    moveToNextToken();
    return num;
}

size_t ConfigParser::parseSizeT()
{
    expect(DATA);
    std::istringstream iss(currentToken().data);
    size_t num;
    iss >> num;
    if (iss.fail() || !iss.eof())
        throw ConfigException("Error: Invalid size_t value", _config_input, _tokens[_current].offset);
    moveToNextToken();
    return num;
}

std::map<int, std::string> ConfigParser::parseErrorRedirects()
{
    std::map<int, std::string> errorRedirects;
    expect(DATA);
    std::istringstream iss(currentToken().data);
    int errorNum;
    iss >> errorNum;
    if (iss.fail() || !iss.eof())
        throw ConfigException("Error: Invalid error number", _config_input, _tokens[_current].offset);
    moveToNextToken();
    expect(DATA);
    std::string redirect;
    redirect = currentToken().data;
    moveToNextToken();
    errorRedirects[errorNum] = redirect;
    return errorRedirects;
}

// Parse LocalRouteConfig
LocalRouteConfig ConfigParser::parseLocalRouteConfig(ServerConfig &serverConfig)
{
    LocalRouteConfig localRouteConfig;
    std::map<std::string, std::string> currentCgiFileExtension;

    moveToNextToken();
    localRouteConfig.path = parseLocalRoutePath();
    isRedundantToken(_tokens[_current - 1].offset, serverConfig, KW_LOCATION,
                     localRouteConfig.path, _config_input);
    expect(SY_BRACE_OPEN);
    while (currentToken().kind != SY_BRACE_CLOSE)
    {
        switch (currentToken().kind)
        {
        case SY_COMMEND:
            moveToNextToken();
            break;

        case KW_ROOT:
            isRedundantToken(_tokens[_current].offset, localRouteConfig, KW_ROOT, _config_input);
            isRedundantToken(_tokens[_current].offset, localRouteConfig, KW_REDIRECT_ADDRESS, _config_input);
            moveToNextToken();
            localRouteConfig.rootDirectory = parseString();
            expect(SY_SEMICOLON);
            break;
        case KW_ALLOW_METHODS:
            isRedundantToken(_tokens[_current].offset, localRouteConfig, KW_ALLOW_METHODS, _config_input);
            moveToNextToken();
            localRouteConfig.allowedMethods =
                parseAllowedHttpMethods(serverConfig, localRouteConfig);
            expect(SY_SEMICOLON);
            break;
        case KW_AUTOINDEX:
            isRedundantToken(_tokens[_current].offset, localRouteConfig, KW_AUTOINDEX, _config_input);
            moveToNextToken();
            localRouteConfig.allowListing = parseDirectoryListing();
            expect(SY_SEMICOLON);
            break;
        case KW_INDEX:
            isRedundantToken(_tokens[_current].offset, localRouteConfig, KW_INDEX, _config_input);
            moveToNextToken();
            localRouteConfig.indexFile = parseString();
            expect(SY_SEMICOLON);
            break;
        case KW_CGI:
            moveToNextToken();
            currentCgiFileExtension = parseCgiFileExtensions();
            isRedundantToken(_tokens[_current - 2].offset, localRouteConfig, KW_CGI,
                             currentCgiFileExtension, _config_input);
            localRouteConfig.cgiTypes.insert(currentCgiFileExtension.begin(),
                                             currentCgiFileExtension.end());
            expect(SY_SEMICOLON);
            break;
        case KW_ALLOW_UPLOAD:
            isRedundantToken(_tokens[_current].offset, localRouteConfig, KW_ALLOW_UPLOAD, _config_input);
            moveToNextToken();
            localRouteConfig.allowUpload = parseAllowUpload();
            expect(SY_SEMICOLON);
            break;
        case KW_REDIRECT_ADDRESS:
            isRedundantToken(_tokens[_current].offset, localRouteConfig, KW_REDIRECT_ADDRESS, _config_input);
            isRedundantToken(_tokens[_current].offset, localRouteConfig, KW_ROOT, _config_input);
            moveToNextToken();
            serverConfig.redirectRoutes.push_back(parseRedirectRouteConfig(localRouteConfig));
            expect(SY_SEMICOLON);
            break;
        default:
            throw ConfigException("Error: Unexpected token in local route config",
                                  _config_input, _tokens[_current].offset);
        }
    }

    expect(SY_BRACE_CLOSE);
    isRouteTokensMissing(localRouteConfig.parsedTokens, _tokens[_current].offset,_config_input);
    return localRouteConfig;
}

std::set<HttpMethod> ConfigParser::parseAllowedHttpMethods(ServerConfig &serverConfig,
                                                           LocalRouteConfig &localRouteConfig)
{
    std::set<HttpMethod> allowedMethods;
    while (currentToken().kind != SY_SEMICOLON)
    {
        expect(DATA);
        HttpMethod method = parseHttpMethod(currentToken().data);
        if (method == HTTP_METHOD_NONE)
            throw ConfigException("Error: Invalid HTTP method", _config_input, _tokens[_current].offset);
        allowedMethods.insert(method);

        // If a redirect route exists for this local route, add the allowed methods to the
        // redirect
        if (serverConfig.redirectRoutes.size() != 0)
        {
            for (std::vector<RedirectRouteConfig>::iterator redirectIt =
                     serverConfig.redirectRoutes.begin();
                 redirectIt != serverConfig.redirectRoutes.end(); ++redirectIt)
            {
                if (redirectIt->path == localRouteConfig.path &&
                    redirectIt->allowedMethods.size() != allowedMethods.size())
                    redirectIt->allowedMethods = allowedMethods;
            }
        }
        moveToNextToken();
    }
    return allowedMethods;
}

bool ConfigParser::parseDirectoryListing()
{
    expect(DATA);
    bool allowDirectoryListing;
    if (currentToken().data == "on")
        allowDirectoryListing = true;
    else if (currentToken().data == "off")
        allowDirectoryListing = false;
    else
        throw ConfigException("Error: Invalid value for autoindex", _config_input, _tokens[_current].offset);
    moveToNextToken();
    return allowDirectoryListing;
}

bool ConfigParser::parseAllowUpload()
{
    expect(DATA);
    bool allowUpload;
    if (currentToken().data == "on")
        allowUpload = true;
    else if (currentToken().data == "off")
        allowUpload = false;
    else
        throw ConfigException("Error: Invalid value for allow_upload", _config_input, _tokens[_current].offset);
    moveToNextToken();
    return allowUpload;
}

std::map<std::string, std::string> ConfigParser::parseCgiFileExtensions()
{
    std::map<std::string, std::string> cgiFileExtensions;
    std::string extension;
    std::string path;
    expect(DATA);
    extension = currentToken().data;
    checkValidCgiFileExtension(extension, _tokens[_current].offset, _config_input);
    // If the extension is .cgi, the path is the same as server root path
    if (extension == ".cgi")
    {
        cgiFileExtensions[extension] = "";
        moveToNextToken();
        return cgiFileExtensions;
    }
    moveToNextToken();
    expect(DATA);
    path = currentToken().data;
    moveToNextToken();
    cgiFileExtensions[extension] = path;
    return cgiFileExtensions;
}

// Parse RedirectRouteConfig
RedirectRouteConfig ConfigParser::parseRedirectRouteConfig(LocalRouteConfig &localRouteConfig)
{
    RedirectRouteConfig redirectRouteConfig;
    expect(DATA);
    redirectRouteConfig.redirectLocation = currentToken().data;
    redirectRouteConfig.path = localRouteConfig.path;
    if (localRouteConfig.allowedMethods.size() != 0)
        redirectRouteConfig.allowedMethods = localRouteConfig.allowedMethods;
    moveToNextToken();
    return redirectRouteConfig;
}

ApplicationConfig ConfigParser::createConfig(const char *path)
{
    std::string config_input = readFile(path);
    ConfigTokenizer tokenizer(config_input);
    std::vector<Token> tokens = tokenizer.tokenize();
    // tokenizer.printTokens(tokens);

    if (tokens.size() == 0)
        throw ConfigException("Error: Empty config file", config_input, 0);

    ConfigParser parser(tokens);
    parser._config_input = config_input;

    ApplicationConfig config = parser.parse();
    return config;
}

/* Constructs a parser exception using the given reason, source and offset */
ConfigException::ConfigException(const std::string &reason, const std::string &source, size_t offset)
    : _reason(reason)
    , _source(source)
    , _offset(offset)
{
}

/* Constructs a parser exception using the given reason and offset */
ConfigException::ConfigException(const std::string &reason, size_t offset)
    : _reason(reason)
    , _source()
    , _offset(offset)
{
}

/* Constructs a parser exception using the given reason */
ConfigException::ConfigException(const std::string &reason)
    : _reason(reason)
{
}

/* Empty method; declares the destructor as non-throwing */
ConfigException::~ConfigException() throw() {}

/* Gets the cause of the error as a C-style string */
const char *ConfigException::what() const throw()
{
    return _reason.c_str();
}

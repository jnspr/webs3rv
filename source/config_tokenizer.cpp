#include "config_parser.hpp"
#include "config_tokenizer.hpp"

// Token
Token::Token(TokenKind kind, std::string data, size_t offset)
    : kind(kind), data(data), offset(offset)
{
}

// Constructor
ConfigTokenizer::ConfigTokenizer(const std::string &config_input)
    : config_input(config_input), offset(0)
{
}

// Methods

std::vector<Token> ConfigTokenizer::tokenize()
{
    std::vector<Token> tokens;

    while (offset < config_input.length())
    {
        if (isspace(config_input[offset]))
            offset++;
        else if (config_input[offset] == '{')
        {
            tokens.push_back(Token(SY_BRACE_OPEN, "{", offset));
            offset++;
        }
        else if (config_input[offset] == '}')
        {
            tokens.push_back(Token(SY_BRACE_CLOSE, "}", offset));
            offset++;
        }
        else if (config_input[offset] == ';')
        {
            tokens.push_back(Token(SY_SEMICOLON, ";", offset));
            offset++;
        }
        else if (isalnum(config_input[offset]) || config_input[offset] == '/' ||
                 config_input[offset] == '.' || config_input[offset] == '_')
            tokens.push_back(parseKeywordOrData());
        else if (config_input[offset] == '#')
            tokens.push_back(parseComment());
        else
        {
            throw ConfigException("Invalid token", config_input, offset);
        }
    }

    return (tokens);
}

Token ConfigTokenizer::parseKeywordOrData()
{
    size_t start = offset;
    while (offset < config_input.length() && !isspace(config_input[offset]) &&
           config_input[offset] != '{' && config_input[offset] != '}' &&
           config_input[offset] != ';' && config_input[offset] != '#')
    {
        offset++;
    }
    std::string word = config_input.substr(start, offset - start);
    TokenKind kind = getTokenKind(word);
    return (Token(kind, word, start));
}

Token ConfigTokenizer::parseComment()
{
    size_t start = offset;
    while (offset < config_input.length() && config_input[offset] != '\n')
    {
        offset++;
    }
    std::string word = config_input.substr(start, offset - start);
    TokenKind kind = getTokenKind("#");
    return (Token(kind, word, start));
}

TokenKind ConfigTokenizer::getTokenKind(const std::string &word)
{
    if (word == "server")
        return (KW_SERVER);
    else if (word == "location")
        return (KW_LOCATION);
    else if (word == "rewrite")
        return (KW_REDIRECT_ADDRESS);
    else if (word == "listen")
        return (KW_PORT);
    else if (word == "server_name")
        return (KW_SERVER_NAME);
    else if (word == "error_page")
        return (KW_ERROR_PAGE);
    else if (word == "root")
        return (KW_ROOT);
    else if (word == "autoindex")
        return (KW_AUTOINDEX);
    else if (word == "index")
        return (KW_INDEX);
    else if (word == "allow_methods")
        return (KW_ALLOW_METHODS);
    else if (word == "max_body_size")
        return (KW_MAX_BODY_SIZE);
    else if (word == "cgi")
        return (KW_CGI);
    else if (word == "allow_upload")
        return (KW_ALLOW_UPLOAD);
    else if (word == "#")
        return (SY_COMMEND);
    else
        return (DATA);
}

void ConfigTokenizer::printTokens(std::vector<Token> tokens)
{
    for (size_t i = 0; i < tokens.size(); i++)
    {
        // print token kind as enum keyword sting
        std::cout << "Token " << i << ": "
                  << "offset= " << tokens[i].offset << " kind= " << tokens[i].kind
                  << " kind_as_string: \t" << TokenKindToString(tokens[i].kind)
                  << ",\t data = " << tokens[i].data << std::endl;
    }
    std::cout << "Finished printing tokens" << std::endl
              << std::endl;
}

std::string ConfigTokenizer::TokenKindToString(TokenKind token)
{
    switch (token)
    {
    case KW_SERVER:
        return "KW_SERVER";
    case KW_LOCATION:
        return "KW_LOCATION";
    case KW_REDIRECT_ADDRESS:
        return "KW_REDIRECT_ADDRESS";
    case KW_PORT:
        return "KW_PORT";
    case KW_SERVER_NAME:
        return "KW_SERVER_NAME";
    case KW_ERROR_PAGE:
        return "KW_ERROR_PAGE";
    case KW_ROOT:
        return "KW_ROOT";
    case KW_AUTOINDEX:
        return "KW_AUTOINDEX";
    case KW_INDEX:
        return "KW_INDEX";
    case KW_ALLOW_METHODS:
        return "KW_ALLOW_METHODS";
    case KW_MAX_BODY_SIZE:
        return "KW_BODY_SIZE";
    case KW_CGI:
        return "KW_CGI";
    case KW_ALLOW_UPLOAD:
        return "KW_ALLOW_UPLOAD";
    case SY_BRACE_OPEN:
        return "SY_BRACE_OPEN";
    case SY_BRACE_CLOSE:
        return "SY_BRACE_CLOSE";
    case SY_SEMICOLON:
        return "SY_SEMICOLON";
    case SY_COMMEND:
        return "SY_COMMEND";
    case DATA:
        return "DATA";
    default:
        return "Unknown TokenKind";
    }
}

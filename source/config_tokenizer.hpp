#ifndef CONFIG_TOKENIZER_hpp
#define CONFIG_TOKENIZER_hpp

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

enum TokenKind
{
    KW_SERVER,
    KW_LOCATION,
    KW_REDIRECT_ADDRESS,
    KW_PORT,
    KW_SERVER_NAME,
    KW_ERROR_PAGE,
    KW_ROOT,
    KW_AUTOINDEX,
    KW_INDEX,
    KW_ALLOW_METHODS,
    KW_MAX_BODY_SIZE,
    KW_CGI,
    SY_BRACE_OPEN,
    SY_BRACE_CLOSE,
    SY_SEMICOLON,
    SY_COMMEND,
    DATA,
};

struct Token
{
    TokenKind kind;
    std::string data;
    size_t offset;

    Token(TokenKind kind, std::string data, size_t offset);
};

struct ConfigTokenizer
{
    // Members
    std::string config_input;
    size_t offset;

    // Constructor
    ConfigTokenizer(const std::string &config_input);

    // Methods
    std::vector<Token> tokenize();
    Token parseKeywordOrData();
    Token parseComment();
    TokenKind getTokenKind(const std::string &word);
    void printTokens(std::vector<Token> tokens);
    std::string TokenKindToString(TokenKind kind);

    // Exceptions
    struct TokenazierException : public std::exception
    {
        mutable std::string msg;
        size_t offset;
        std::string config_input;

        TokenazierException(size_t offset, const std::string &config_input);
        ~TokenazierException() throw();
        virtual const char *what() const throw();
    };
};

#endif // CONFIG_TOKENIZER_hpp

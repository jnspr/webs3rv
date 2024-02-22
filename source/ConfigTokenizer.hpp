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

#ifndef CONFIGTOKENIZER_HPP
#define CONFIGTOKENIZER_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

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

#endif
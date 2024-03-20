#include "error_db.h"

/* Constructs an error database with the default entries */
ErrorDB::ErrorDB(): _defaultError("Unknown Error")
{
    _entries[200] = "OK";
    _entries[404] = "Not Found";
    _entries[500] = "Internal Server Error";
    _entries[501] = "Not Implemented";
    _entries[502] = "Bad Gateway";
    _entries[503] = "Service Unavailable";
    _entries[504] = "Gateway Timeout";
    _entries[505] = "HTTP Version Not Supported";
    _entries[506] = "Variant Also Negotiates";
    _entries[507] = "Insufficient Storage";
    _entries[508] = "Loop Detected";
    _entries[510] = "Not Extended";
    _entries[511] = "Network Authentication Required";
    _entries[103] = "Early Hints";
    _entries[100] = "Continue";
    _entries[101] = "Switching Protocols";
    _entries[102] = "Processing";
    _entries[203] = "Non-Authoritative Information";
    _entries[204] = "No Content";
    _entries[205] = "Reset Content";
    _entries[206] = "Partial Content";
    _entries[207] = "Multi-Status";
    _entries[208] = "Already Reported";
    _entries[226] = "IM Used";
    _entries[300] = "Multiple Choices";
    _entries[301] = "Moved Permanently";
    _entries[302] = "Found";
    _entries[303] = "See Other";
    _entries[304] = "Not Modified";
    _entries[305] = "Use Proxy";
    _entries[307] = "Temporary Redirect";
    _entries[308] = "Permanent Redirect";
    _entries[400] = "Bad Request";
    _entries[401] = "Unauthorized";
    _entries[402] = "Payment Required";
    _entries[403] = "Forbidden";
    _entries[406] = "Not Acceptable";
    _entries[407] = "Proxy Authentication Required";
    _entries[408] = "Request Timeout";
    _entries[409] = "Conflict";
    _entries[410] = "Gone";
    _entries[411] = "Length Required";
    _entries[412] = "Precondition Failed";
    _entries[413] = "Payload Too Large";
    _entries[414] = "URI Too Long";
    _entries[415] = "Unsupported Media Type";
    _entries[416] = "Range Not Satisfiable";
    _entries[417] = "Expectation Failed";
    _entries[418] = "I'm a teapot";
    _entries[421] = "Misdirected Request";
    _entries[422] = "Unprocessable Entity";
    _entries[423] = "Locked";
    _entries[424] = "Failed Dependency";
    _entries[425] = "Too Early";
    _entries[426] = "Upgrade Required";
    _entries[428] = "Precondition Required";
    _entries[429] = "Too Many Requests";
    _entries[431] = "Request Header Fields Too Large";
    _entries[451] = "Unavailable For Legal Reasons";
}

/* Gets the error message for the error number specified */
const std::string &ErrorDB::getErrorType(int code)
{
    std::map<int, std::string>::const_iterator iterator = find(_entries.begin(), _entries.end(), code);
    if (iterator != _entries.end())
        return iterator->second;
    return _defaultError;
}

ErrorDB g_errorDB;

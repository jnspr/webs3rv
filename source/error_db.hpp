#ifndef ERROR_DB_HPP
#define ERROR_DB_HPP

#include <map>
#include <string>

class ErrorDB {

public:
    /* Constructs an error database with the default entries */
    ErrorDB();

    /* Gets the error message for the error number specified */
    const std::string &getErrorType(int code);
private:
    std::map<int, std::string>         _entries;
    std::string                        _defaultError;
};

extern ErrorDB g_errorDB;

#endif // ERROR_DB_HPP

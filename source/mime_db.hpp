#ifndef MIME_DB_HPP
#define MIME_DB_HPP

#include <map>
#include <string>

#include "slice.hpp"

class MimeDB
{
public:
    /* Constructs a mime database with the default entries */
    MimeDB();

    /* Gets the mime type of a file path according to the entries */
    const std::string &getMimeType(Slice path);
private:
    std::map<std::string, std::string> _entries;
    std::string                        _defaultType;
};

/* The global instance of the mime type database */
extern MimeDB g_mimeDB;

#endif // MIME_DB_HPP
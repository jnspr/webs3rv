#include "mime_db.hpp"

/* Constructs a mime database with the default entries */
MimeDB::MimeDB(): _defaultType("application/octet-stream")
{
    _entries["html"] = "text/html";
    _entries["htm"] = "text/html";
    _entries["css"] = "text/css";
    _entries["js"] = "text/javascript";
    _entries["jpg"] = "image/jpeg";
    _entries["jpeg"] = "image/jpeg";
    _entries["png"] = "image/png";
    _entries["gif"] = "image/gif";
    _entries["bmp"] = "image/bmp";
    _entries["ico"] = "image/x-icon";
    _entries["svg"] = "image/svg+xml";
    _entries["mp3"] = "audio/mpeg";
    _entries["mp4"] = "video/mp4";
    _entries["webm"] = "video/webm";
    _entries["ogg"] = "audio/ogg";
    _entries["wav"] = "audio/wav";
    _entries["flac"] = "audio/flac";
    _entries["json"] = "application/json";
    _entries["xml"] = "application/xml";
    _entries["pdf"] = "application/pdf";
    _entries["zip"] = "application/zip";
    _entries["tar"] = "application/x-tar";
    _entries["gz"] = "application/gzip";
    _entries["rar"] = "application/x-rar-compressed";
    _entries["7z"] = "application/x-7z-compressed";
    _entries["exe"] = "application/x-msdownload";
    _entries["psd"] = "image/vnd.adobe.photoshop";
    _entries["ai"] = "application/postscript";
    _entries["eps"] = "application/postscript";
    _entries["rtf"] = "application/rtf";
    _entries["txt"] = "text/plain";
    _entries["csv"] = "text/csv";
    _entries["avi"] = "video/x-msvideo";
    _entries["mpeg"] = "video/mpeg";
    _entries["mpg"] = "video/mpeg";
    _entries["wmv"] = "video/x-ms-wmv";
    _entries["webp"] = "image/webp";
    _entries["tiff"] = "image/tiff";
    _entries["tif"] = "image/tiff";
    _entries["ps"] = "application/postscript";
    _entries["eot"] = "application/vnd.ms-fontobject";
    _entries["otf"] = "font/otf";
    _entries["ttf"] = "font/ttf";
    _entries["woff"] = "font/woff";
    _entries["woff2"] = "font/woff2";
    _entries["flv"] = "video/x-flv";
    _entries["midi"] = "audio/midi";
    _entries["mid"] = "audio/midi";
    _entries["weba"] = "audio/webm";
    _entries["webm"] = "video/webm";
    _entries["webp"] = "image/webp";
    _entries["mkv"] = "video/x-matroska";
}

/* Gets the mime type of a file path according to the entries */
const std::string &MimeDB::getMimeType(Slice path)
{
    // FIXME: This is O(n), could be faster by stripping off extension and using .find()
    std::map<std::string, std::string>::const_iterator iterator = _entries.begin();
    for (; iterator != _entries.end(); iterator++)
    {
        if (path.endsWith(iterator->first))
            return iterator->second;
    }
    return _defaultType;
}

/* The global instance of the mime type database */
MimeDB g_mimeDB;

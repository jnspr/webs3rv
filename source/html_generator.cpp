#include "html_generator.hpp"
#include "http_exception.hpp"
#include "utility.hpp"
#include "error_db.hpp"

#include <dirent.h>
#include <strings.h>
#include <algorithm>

/* RAII wrapper around `opendir()` and `closedir()` */
class DirectoryHandle
{
public:
    /* Opens a directory pointer */
    inline DirectoryHandle(const char *path)
    {
        if ((_pointer = opendir(path)) == NULL)
            throw std::runtime_error("Unable to open directory");
    }

    /* Releases the directory pointer */
    inline ~DirectoryHandle()
    {
        if (_pointer)
            closedir(_pointer);
    }

    /* Gets the next file name */
    inline dirent *next()
    {
        return readdir(_pointer);
    }
private:
    DIR *_pointer;

    /* Prevent construction by copy */
    DirectoryHandle(const DirectoryHandle &other);

    /* Prevent assignment by copy */
    DirectoryHandle &operator=(const DirectoryHandle &other);
};

/* Comparison function for `std::sort()` */
static bool compareLowercase(const std::string &lhs, const std::string &rhs)
{
#ifdef __42_LIKES_WASTING_CPU_CYCLES__
    const char *lhsPointer = lhs.c_str();
    const char *rhsPointer = rhs.c_str();

    while (*lhsPointer && *rhsPointer)
    {
        int lhsCharacter = std::tolower(*lhsPointer);
        int rhsCharacter = std::tolower(*rhsPointer);
        if (lhsCharacter != rhsCharacter)
            return (lhsCharacter - rhsCharacter) < 0;
        lhsPointer++;
        rhsPointer++;
    }
    return (std::tolower(*lhsPointer) - std::tolower(*rhsPointer)) < 0;
#else
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
#endif // __42_LIKES_WASTING_CPU_CYCLES__
}

/* Generates an error page for the given status code */
std::string HtmlGenerator::errorPage(int statusCode)
{
    std::stringstream  stream;
    const std::string &message = g_errorDB.getErrorType(statusCode);

    stream << "<!DOCTYPE html>\n"
              "<html lang=\"en\">\n"
              "<head>\n"
              "    <meta charset=\"UTF-8\">\n"
              "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
              "    <title>" << statusCode << " - " << message << "</title>\n"
              "</head>\n"
              "<body>\n"
              "    <h1>" << statusCode << " - " << message << "</h1>\n"
              "    <hr>"
              "    <p>This is the default error page</p>"
              "</body>\n"
              "</html>";

    return stream.str();
}

/* Generates a directory list page for the given directory */
std::string HtmlGenerator::directoryList(const char *path)
{
    dirent                  *entry;
    std::vector<std::string> files;
    std::stringstream        stream;
    DirectoryHandle          handle(path);

    stream << "<!DOCTYPE html>\n"
              "<html lang=\"en\">\n"
              "<head>\n"
              "    <meta charset=\"UTF-8\">\n"
              "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
              "    <title>Directory listing</title>\n"
              "    <style>\n"
              "        body {\n"
              "            font-family: Arial, sans-serif;\n"
              "        }\n"
              "    </style>\n"
              "</head>\n"
              "<body>\n"
              "<h1>Directory listing</h1>\n"
              "<hr>\n"
              "<ul>";

    while ((entry = handle.next()) != NULL)
        files.push_back(entry->d_name);

    std::sort(files.begin(), files.end(), compareLowercase);

    std::vector<std::string>::iterator iterator = files.begin();
    for (; iterator != files.end(); iterator++)
        stream << "<li><a href=\"" << *iterator << "\">" << *iterator << "</a></li>\n";

    stream << "</ul>\n"
              "\n"
              "</body>\n"
              "</html>\n";

    return stream.str();
}

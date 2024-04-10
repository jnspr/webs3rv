#ifndef HTML_GENERATOR_hpp
#define HTML_GENERATOR_hpp

#include <string>

namespace HtmlGenerator
{
    /* Generates an error page for the given status code */
    std::string errorPage(int statusCode);

    /* Generates a directory list page for the given directory */
    std::string directoryList(const char *path);
};

#endif // HTML_GENERATOR_hpp

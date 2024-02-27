#ifndef DEBUG_UTILITY_hpp
#define DEBUG_UTILITY_hpp

#include "config.hpp"
#include "http_request.hpp"

namespace Debug
{
    /* Prints the given configuration to standard output */
    void printConfig(const ApplicationConfig &config);

    /* Prints the given HTTP request to standard output */
    void printRequest(const HttpRequest &request);
};

#endif // DEBUG_UTILITY_hpp

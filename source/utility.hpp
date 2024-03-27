#ifndef UTILITY_hpp
#define UTILITY_hpp

#include "slice.hpp"
#include "config.hpp"

#include <string>
#include <cstddef>
#include <sstream>
#include <stdint.h>
#include <cstdio> 
#include <netdb.h>
#include <vector>


enum NodeType
{
    NODE_TYPE_REGULAR,
    NODE_TYPE_DIRECTORY,
    NODE_TYPE_NOT_FOUND,
    NODE_TYPE_NO_ACCESS,
    /* Symlinks, sockets, named pipes, etc. */
    NODE_TYPE_UNSUPPORTED
};

namespace Utility
{
    /* Converts a number to a string */
    template <typename T>
    static std::string numberToString(T number)
    {
        std::stringstream ss;
        ss << number;
        return ss.str();
    };

    /* Queries the type of node at the given FS path */
    NodeType queryNodeType(const char *path);

    /* Queries the type of node at the given FS path */
    static inline NodeType queryNodeType(const std::string &path)
    {
        return queryNodeType(path.c_str());
    }

    /* Checks whether the given path exceeds the root path, returns true if it doesn't */
    bool checkPathLevel(Slice path);

    /* Converts a uint32_t ipv4 adress to a string*/
    std::string ipv4ToString(uint32_t ipv4Addr);

    /* Attempts to convert a string slice to a `size_t` */
    bool parseSize(Slice string, size_t &outResult);

    /* Attempts to convert a hexadecimal character (0-9, A-F, a-f) into its numeric value (0-15) */
    bool parseHexChar(char character, uint8_t &outResult);

    /* Attempts to convert a string slice to a `size_t` */
    bool parseSizeHex(Slice string, size_t &outResult);

    /* Attempts to convert a URL-encoded string slice to a URL-decoded string */
    bool decodeUrl(Slice string, std::string &outResult);

    /* Checks the config for more than one server listening on the same address*/
    void checkduplicatehost(const ApplicationConfig &config);


    /* Class to resolve an host Adress and return the ipv4*/
    class AddrInfo {
        public:
            AddrInfo(const char *hostname);
            ~AddrInfo();
            /* Returns a vector with the resolved http Adresses*/
            std::vector<struct addrinfo *> getResult() const;


        private:
            const char      *_hostname;
            const char      *_service;
            struct addrinfo *_result;
    };
}

#endif // UTILITY_hpp

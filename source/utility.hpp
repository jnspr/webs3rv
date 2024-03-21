#ifndef UTILITY_hpp
#define UTILITY_hpp

#include "slice.hpp"

#include <string>
#include <cstddef>
#include <sstream>
#include <stdint.h>
#include <cstdio> 

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

    /* Attempts to convert a string slice to a `size_t` */
    bool parseSizeHex(Slice string, size_t &outResult);
}

#endif // UTILITY_hpp

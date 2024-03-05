#ifndef UTILITY_hpp
#define UTILITY_hpp

#include "slice.hpp"

#include <string>
#include <sstream>

enum NodeType
{
    /* The file can't be accessed */
    NODE_TYPE_ERROR,
    NODE_TYPE_REGULAR,
    NODE_TYPE_DIRECTORY,
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
}

#endif // UTILITY_hpp

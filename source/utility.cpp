#include "utility.hpp"

#include <iostream>
#include <sys/stat.h>

/* Queries the type of node at the given FS path */
NodeType Utility::queryNodeType(const char *path)
{
    struct stat result;

    if (stat(path, &result) != 0)
        return NODE_TYPE_ERROR;

    if (S_ISREG(result.st_mode))
        return NODE_TYPE_REGULAR;
    if (S_ISDIR(result.st_mode))
        return NODE_TYPE_DIRECTORY;
    return NODE_TYPE_UNSUPPORTED;
}

/* Checks whether the given path exceeds the root path, returns true if it doesn't */
bool Utility::checkPathLevel(Slice path)
{
    Slice  node;
    size_t level = 0;
    bool   isLastNode = false;

    // Get rid of leading slashes
    path.stripStart('/');

    while (!isLastNode)
    {
        // Treat the trailing part as another node
        if (!path.splitStart('/', node))
        {
            node = path;
            isLastNode = true;
        }
        // Don't allow the path to escape the root
        if (node == C_SLICE(".."))
        {
            if (level == 0)
                return false;
            level--;
        }
        // All nodes except '.' or empty ones increase the level
        else if (!node.isEmpty() && node != C_SLICE("."))
            level++;
    }

    return true;
}

#include "utility.hpp"

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

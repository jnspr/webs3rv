#include "utility.hpp"

#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>

/* Queries the type of node at the given FS path */
NodeType Utility::queryNodeType(const char *path)
{
    struct stat result;

    if (stat(path, &result) != 0)
    {
        if (errno == EACCES)
            return NODE_TYPE_NO_ACCESS;
        if (errno == ENOENT)
            return NODE_TYPE_NOT_FOUND;
        throw std::runtime_error("Unable to query file information");
    }

    // Separately check for permissions on the node since stat() doesn't account for this
    if (access(path, R_OK) != 0)
        return NODE_TYPE_NO_ACCESS;

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

/* Converts a uint32_t ipv4 adress to a string*/
std::string Utility::ipv4ToString(uint32_t ipv4Addr)
{
    char addrBuffer[16];
    sprintf(addrBuffer, "%u.%u.%u.%u", (ipv4Addr >> 24) & 0xFF, (ipv4Addr >> 16) & 0xFF,
        (ipv4Addr >> 8) & 0xFF, ipv4Addr & 0xFF);
    return std::string(addrBuffer);
}

/* Attempts to convert a string slice to a `size_t` */
bool Utility::parseSize(Slice string, size_t &outResult)
{
    char   current;
    size_t digit;
    size_t result = 0;

    // Catch empty strings
    if (string.isEmpty())
        return false;

    for (size_t index = 0; index < string.getLength(); index++)
    {
        // Check if the character is a digit
        current = string[index];
        if (current < '0' || current > '9')
            return false;

        // Shift the result to the left (decimal) and check overflow
        if (result != 0 && SIZE_MAX / result < 10)
            return false;
        result *= 10;

        // Add the digit and check overflow
        digit = static_cast<unsigned char>(current - '0');
        if (SIZE_MAX - result < digit)
            return false;
        result += digit;
    }

    outResult = result;
    return true;
}

/* Attempts to convert a string slice to a `size_t` */
bool Utility::parseSizeHex(Slice string, size_t &outResult)
{
    char   current;
    size_t digit;
    size_t result = 0;

    // Catch empty strings
    if (string.isEmpty())
        return false;

    for (size_t index = 0; index < string.getLength(); index++)
    {
        current = string[index];

        // Check if the character is a hex digit and transform it
        if (current >= '0' && current <= '9')
            digit = static_cast<unsigned char>(current - '0');
        else if (current >= 'A' && current <= 'F')
            digit = static_cast<unsigned char>(current - 'A' + 10);
        else if (current >= 'a' && current <= 'f')
            digit = static_cast<unsigned char>(current - 'a' + 10);
        else
            return false;

        // Shift the result to the left and check overflow
        if (index >= sizeof(size_t) * 2)
            return false;
        result = (result << 4) | digit;
    }

    outResult = result;
    return true;
}

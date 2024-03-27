#include "utility.hpp"

#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <sys/stat.h>
#include <cstring>
#include <netdb.h>

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

/* Attempts to convert a hexadecimal character (0-9, A-F, a-f) into its numeric value (0-15) */
bool Utility::parseHexChar(char character, uint8_t &outResult)
{
    if (character >= '0' && character <= '9')
        outResult = static_cast<unsigned char>(character - '0');
    else if (character >= 'A' && character <= 'F')
        outResult = static_cast<unsigned char>(character - 'A' + 10);
    else if (character >= 'a' && character <= 'f')
        outResult = static_cast<unsigned char>(character - 'a' + 10);
    else
        return false;
    return true;
}

/* Attempts to convert a string slice to a `size_t` */
bool Utility::parseSizeHex(Slice string, size_t &outResult)
{
    uint8_t digit;
    size_t  result = 0;

    // Catch empty strings
    if (string.isEmpty())
        return false;

    for (size_t index = 0; index < string.getLength(); index++)
    {
        // Check if the character is a hex digit and transform it
        if (!parseHexChar(string[index], digit))
            return false;

        // Shift the result to the left and check overflow
        if (index >= sizeof(size_t) * 2)
            return false;
        result = (result << 4) | static_cast<size_t>(digit);
    }

    outResult = result;
    return true;
}

/* Attempts to convert a URL-encoded string slice to a URL-decoded string */
bool Utility::decodeUrl(Slice string, std::string &outResult)
{
    std::stringstream stream;
    uint8_t           upperFour, lowerFour;

    for (size_t index = 0; index < string.getLength(); index++)
    {
        // Handle special characters
        if (string[index] == '%')
        {
            // Check if there are enough characters left (2 hex digits after '%')
            if (string.getLength() - index < 3)
                return false;

            // Parse the two hex digits
            if (!parseHexChar(string[index + 1], upperFour))
                return false;
            if (!parseHexChar(string[index + 2], lowerFour))
                return false;

            // Combine the digits and write it to the string as a character
            stream << static_cast<char>((upperFour << 4) | lowerFour);
            index += 2; // The last character will be consumed by the loop
        }
        // Handle the '+' character as a space
        else if (string[index] == '+')
            stream << ' ';
        else
            stream << string[index];
    }

    outResult = stream.str();
    return true;
}

void Utility::checkduplicatehost(const ApplicationConfig &config)
{
    for(size_t i = 0; i < config.servers.size(); i++)
    {
        for (size_t j = 0; j < config.servers.size(); j++)
        {
            if(i != j && config.servers[i].host == config.servers[j].host 
                && config.servers[i].port == config.servers[j].port)
                throw std::runtime_error("Multiple servers on the same address");
        }
    }
}

Utility::AddrInfo::AddrInfo(const char *hostname) : _hostname(hostname), _service("http"), _result(NULL){
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(_hostname, _service, &hints, &_result);
    if (status != 0)
        throw std::runtime_error("getaddrinfo failed");

}

Utility::AddrInfo::~AddrInfo(){
    if (_result)
        freeaddrinfo(_result);
}

/* Returns a vector with the resolved http Adresses*/
 std::vector<struct addrinfo *>  Utility::AddrInfo::getResult() const {
    std::vector <struct addrinfo *> addresses;
    for (struct addrinfo *p = _result; p != NULL; p = p->ai_next)
        addresses.push_back(p);

    return addresses;
 }
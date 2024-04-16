#include "config.hpp"
#include "utility.hpp"

/* Initializes a server configuration using the default parameters */
ServerConfig::ServerConfig()
    : maxBodySize(100000)
    , nextEndpoint(NULL)
{
}

/* Initializes a local route configuration using the default parameters */
LocalRouteConfig::LocalRouteConfig()
    : allowUpload(false)
    , allowListing(false)
{
}

/* Searches for the right server configuration based on the name, returns `this` if not found */
const ServerConfig *ServerConfig::findServer(Slice name) const
{
    const ServerConfig *head = this;
    while (head != NULL)
    {
        for (size_t index = 0; index < head->name.size(); index++)
        {
            if (Slice(head->name[index]) == name)
                return head;
        }
        head = head->nextEndpoint;
    }
    return this;
}

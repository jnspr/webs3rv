#include "config.hpp"
#include "utility.hpp"

/* Initializes a server configuration using the default parameters */
ServerConfig::ServerConfig()
    : maxBodySize(100000)
{
}

/* Initializes a local route configuration using the default parameters */
LocalRouteConfig::LocalRouteConfig()
    : allowUpload(false)
    , allowListing(false)
{
}

#include "routing.hpp"

/* Sets the route pointer to the given local route; also sets the status */
void RoutingInfo::setLocalRoute(const LocalRouteConfig *localRouteConfig)
{
    status = ROUTING_STATUS_FOUND_LOCAL;
    opaqueRoute = reinterpret_cast<const void *>(localRouteConfig);
}

/* Sets the route pointer to the given redirect route; also sets the status */
void RoutingInfo::setRedirectRoute(const RedirectRouteConfig *redirectRouteConfig)
{
    status = ROUTING_STATUS_FOUND_REDIRECT;
    opaqueRoute = reinterpret_cast<const void *>(redirectRouteConfig);
}

/* Finds a route on a server configuration using the given query path */
RoutingInfo RoutingInfo::findRoute(const ServerConfig &serverConfig, Slice queryPath)
{
    (void)serverConfig;
    (void)queryPath;
    // TODO: Implement this
    RoutingInfo info;
    return info;
}

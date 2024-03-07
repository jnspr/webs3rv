#ifndef ROUTING_hpp
#define ROUTING_hpp

#include "config.hpp"

#include <string>
#include <stdexcept>

/* Possible results of a route search */
enum RoutingStatus
{
    ROUTING_STATUS_NOT_FOUND,
    ROUTING_STATUS_NO_ACCESS,
    ROUTING_STATUS_FOUND_LOCAL,
    ROUTING_STATUS_FOUND_REDIRECT
};

/* Holds the data obtained during a route search */
class RoutingInfo
{
public:
    RoutingStatus       status;
    std::string         nodePath;
    std::string         cgiInterpreter;
    const ServerConfig *serverConfig;

    /* Gets a pointer to the local route if the state is correct */
    inline const LocalRouteConfig *getLocalRoute() const
    {
        if (status != ROUTING_STATUS_FOUND_LOCAL)
            throw std::logic_error("Use of route pointer mismatches route kind");
        return reinterpret_cast<const LocalRouteConfig *>(opaqueRoute);
    }

    /* Gets a pointer to the redirect route if the state is correct */
    inline const RedirectRouteConfig *getRedirectRoute() const
    {
        if (status != ROUTING_STATUS_FOUND_REDIRECT)
            throw std::logic_error("Use of route pointer mismatches route kind");
        return reinterpret_cast<const RedirectRouteConfig *>(opaqueRoute);
    }

    /* Sets the route pointer to the given local route; also sets the status */
    void setLocalRoute(const LocalRouteConfig *localRouteConfig);

    /* Sets the route pointer to the given redirect route; also sets the status */
    void setRedirectRoute(const RedirectRouteConfig *redirectRouteConfig);

    /* Finds a route on a server configuration using the given query path */
    static RoutingInfo findRoute(const ServerConfig &serverConfig, Slice queryPath);
private:
    const void *opaqueRoute;
};

#endif // ROUTING_hpp

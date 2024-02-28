#include "config.hpp"

/* Finds a matching route for path and returns it in a struct*/
RouteResult ServerConfig::findRoute(Slice path) const
{
    RouteResult result = {};
    size_t longestPrefix = 0;
    result.wasFound = false;

    for (size_t index = 0; index < localRoutes.size(); index++)
    {
        if (path.startsWith(localRoutes[index].path))
        {
            if (localRoutes[index].path.size() > longestPrefix)
            {
                result.wasFound = true;
                result.localRoute = &localRoutes[index];
                result.isRedirect = false;
                result.path = localRoutes[index].path;
                result.redirectRoute = NULL;
            }
        }
    }

    if (result.wasFound == true)
       return result;

    for (size_t index = 0; index < redirectRoutes.size(); index++)
    {
        if (path.startsWith(redirectRoutes[index].path))
        {
            if (redirectRoutes[index].path.size() > longestPrefix)
            {
                result.wasFound = true;
                result.redirectRoute = &redirectRoutes[index];
                result.isRedirect = true;
                result.path = redirectRoutes[index].path;
                result.localRoute = NULL;
            }
        }
    }    
    return result;
}

#ifndef ENDPOINT_HPP
#define ENDPOINT_HPP

#include <stdint.h>
#include <iostream>
#include "utility.hpp"

struct endpoint
{
    uint32_t    host;
    uint16_t    port;

    /* Constructor */
    inline endpoint(uint32_t host, uint16_t port) : host(host), port(port)
    {
    }

    /* Stream output operator */
    friend std::ostream &operator<<(std::ostream &stream, const endpoint &endpoint)
    {
        return stream << Utility::ipv4ToString(endpoint.host) << ':' << endpoint.port;
    }
    
    /* Ceck if the host and port are the same as the other endpoint */
        inline bool operator==(const endpoint &other) const
    {
        return host == other.host && port == other.port;
    }

    /* Ceck if the host and port are different from the other endpoint */
        inline bool operator!=(const endpoint &other) const
    {
        return host != other.host || port != other.port;
    }
};

#endif
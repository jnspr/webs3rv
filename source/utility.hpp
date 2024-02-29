#ifndef UTILITY_hpp
#define UTILITY_hpp

#include <string>
#include <sstream>

/* Converts a number to a string */
template <typename T>
std::string numberToString(T number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}  

#endif
#ifndef WEBSERV_GENERATOR_HPP
#define WEBSERV_GENERATOR_HPP

#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <stdexcept>


class Generator
{
private:

public:
    static std::string ErrorPage(int error_number, std::string error_string);
    static std::string DirectoryList(std::string path);

    static std::string arrangeOutput(std::string basicString, std::vector<dirent *> vector1);
};

#endif // WEBSERV_GENERATOR_HPP
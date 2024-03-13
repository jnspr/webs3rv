#include "application.hpp"
#include "config_parser.hpp"
#include "config_tokenizer.hpp"
#include "debug_utility.hpp"
#include "routing.hpp"

#include <iostream>

void parseupload(std::vector<uint8_t> body, ssize_t contentLength, uploadData &data)
{
    // body.data is used for getting char * from vector maybe need to cast this to char *

    // need to add -- to beginning of boundary for start and -- to the end to end
    // all values from header are in request vector headers (not finished yet)
    (void)contentLength;
    // gets the filesize from the http header via strnstr
    printf("Parsing upload\n");
    printf("Contentlength: %ld\n", contentLength);

    // reads the upload body (file) into a new buffer


    // creates a slice from the bodybuffer
     Slice sliceBod((char *) body.data(), body.size());
   // sliceBuf.splitStart('\n', data.boundary);


   // std::cout << "Boundary: \n" << data.boundary << std::endl;
    std::cout << "slicebuf after first slice: " << sliceBod << std::endl;
   (void) data;
}

int main(int argc, char *argv[])
{
    // If the configuration path was not specified, display usage and quit
    if (argc != 2)
    {
        std::cerr << "usage: " << argv[0] << " <path-to-config>" << std::endl;
        return 1;
    }

    // Parse the configuration
    ApplicationConfig config;
    try
    {
        config = ConfigParser::createConfig(argv[1]);
    }
    catch (const ConfigException &e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << "At offset " << e.getOffset() << std::endl;
        std::cerr << e.getSource()[e.getOffset()] << std::endl;
        std::cerr << e.getSource().substr(e.getOffset()) << std::endl;
        return 1;
    }

    // Start the application using the parsed configuration
    try
    {
        Application application(config);
        application.configure();
        application.mainLoop();
    }
    catch (std::exception &exception)
    {
        std::cerr << "fatal: " << exception.what() << std::endl;
        return 1;
    }
    return 0;
}

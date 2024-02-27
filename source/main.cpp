#include "application.hpp"
#include "config_parser.hpp"
#include "config_tokenizer.hpp"

#include <iostream>

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
    catch (const ConfigParser::ParserException &e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << "At offset " << e.getOffset() << std::endl;
        std::cerr << e.getConfigInput()[e.getOffset()] << std::endl;
        std::cerr << e.getConfigInput().substr(e.getOffset()) << std::endl;
    }
    catch (const ConfigTokenizer::TokenazierException &e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << e.config_input[e.offset] << std::endl;
        std::cerr << e.config_input.substr(e.offset) << std::endl;
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

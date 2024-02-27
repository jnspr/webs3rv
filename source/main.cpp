#include "application.hpp"
#include "config_parser.hpp"
#include "config_tokenizer.hpp"

#include <iostream>

static ApplicationConfig createExampleConfig()
{
    ApplicationConfig applicationConfig = {};
    {
        ServerConfig serverConfig = {};
        serverConfig.name = "example_server";
        serverConfig.host = 0x7f000001;
        serverConfig.port = 8000;
        serverConfig.maxBodySize = 8192;
        serverConfig.errorPages[404] = "404.html";
        {
            LocalRouteConfig routeConfig = {};
            routeConfig.path = "/";
            routeConfig.allowedMethods.insert(HTTP_METHOD_GET);
            routeConfig.allowedMethods.insert(HTTP_METHOD_POST);
            routeConfig.rootDirectory = "./www";
            routeConfig.indexFile = "index.html";
            routeConfig.allowUploads = false;
            routeConfig.allowListing = true;
            routeConfig.cgiTypes["py"] = "/usr/bin/python3";
            serverConfig.localRoutes.push_back(routeConfig);
        }
        applicationConfig.servers.push_back(serverConfig);
    }
    return applicationConfig;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    ApplicationConfig exampleConfig = createExampleConfig();
    ApplicationConfig config;
    try
    {
        config = ConfigParser::createConfig("/home/cgodecke/Desktop/Core/JPwebs3rv/source/server.conf");
        printConfig(config);
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

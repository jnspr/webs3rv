#ifndef CGI_PROCESS_hpp
#define CGI_PROCESS_hpp

#include "process.hpp"
#include "dispatcher.hpp"
#include "http_client.hpp"
#include "http_request.hpp"
#include "utility.hpp"
#include "routing.hpp"

#include <stdint.h>
#include <stddef.h>

#define SUBSCRIBE_FLAG_INPUT  (1 << 0)
#define SUBSCRIBE_FLAG_OUTPUT (1 << 1)

class HttpClient;

enum CgiProcessState
{
    CGI_PROCESS_RUNNING,
    CGI_PROCESS_FAILURE,
    CGI_PROCESS_TIMEOUT,
    CGI_PROCESS_SUCCESS,
};

struct CgiPathInfo
{
    std::string workingDirectory;
    std::string fileName;

    CgiPathInfo(const std::string &nodePath);
};

class CgiProcess: public Sink
{
public:
    friend class Application;
    friend class HttpClient;

    /* Constructs a CGI process from the given client, request and route result */
    CgiProcess(HttpClient *client, const HttpRequest &request, const RoutingInfo &routingInfo);

    /* Destroys the process */
    ~CgiProcess();

    /* Handles one or multiple events */
    void handleEvents(uint32_t eventMask);

    /* Handles an exception that occurred in `handleEvent()` */
    void handleException(const char *message);

    /* Gets the process state */
    inline CgiProcessState getState()
    {
        return _state;
    }

    /* Sets the process state */
    inline void setState(CgiProcessState state)
    {
        _state = state;
    }

    /* Gets the underlying process */
    inline Process &getProcess()
    {
        return _process;
    }

    /* Gets the associated timeout object */
    inline Timeout &getTimeout()
    {
        return _timeout;
    }

    /* Transitions the process into timeout state */
    void handleTimeout();
private:
    CgiProcessState      _state;
    CgiPathInfo          _pathInfo;
    HttpClient          *_client;
    const HttpRequest   &_request;
    Process              _process;
    std::vector<uint8_t> _buffer;
    Timeout              _timeout;
    size_t               _bodyOffset;
    unsigned int         _subscribeFlags;

    /* Creates a vector of strings for the process arguments */
    static std::vector<std::string> setupArguments(const HttpRequest &request, const RoutingInfo &routingInfo, const std::string &fileName);

    /* Creates a vector of strings for the process environment */
    static std::vector<std::string> setupEnvironment(const HttpRequest &request, const RoutingInfo &routingInfo);
};

#endif // CGI_PROCESS_hpp

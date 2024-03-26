#ifndef EVENT_DISPATCHER_hpp
#define EVENT_DISPATCHER_hpp

#include <vector>
#include <stdint.h>
#include <stdexcept>
#include <sys/epoll.h>

/* Alias for a vector of epoll events */
typedef std::vector<epoll_event> EventBuffer;

struct Sink
{
    /* Handles one or multiple events */
    virtual void handleEvents(uint32_t eventMask) = 0;

    /* Handles an exception that occurred in `handleEvent()` */
    virtual void handleException(const char *message) = 0;

    /* Destructor for deriving classes */
    virtual ~Sink();
};

class Dispatcher
{
public:
    /* Constructs an event dispatcher using the given buffer size */
    Dispatcher(size_t bufferSize);

    /* Releases the dispatcher's resources */
    ~Dispatcher();

    /* Subscribes the an event sink to receive the given events on a file descriptor */
    void subscribe(int fileno, uint32_t eventMask, Sink *sink);

    /* Changes the given file descriptor's received events and event sink */
    void modify(int fileno, uint32_t eventMask, Sink *sink);

    /* Unsubscribes the given file descriptor's event sink from receiving events */
    void unsubscribe(int fileno);

    /* Waits (in the given timeout) for events to occur and dispatches them */
    void dispatch(int timeout = -1);
private:
    EventBuffer _buffer;
    size_t      _bufferSize;
    int         _epollFileno;

    /* Disable copy-construction and copy-assignment */
    Dispatcher(const Dispatcher &other);
    Dispatcher &operator=(const Dispatcher &other);
};

#endif // EVENT_DISPATCHER_hpp

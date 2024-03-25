#include "dispatcher.hpp"

#include <unistd.h>
#include <stdexcept>

/* Destructor for deriving classes */
Sink::~Sink()
{
}

/* Constructs an event dispatcher using the given buffer size */
Dispatcher::Dispatcher(size_t bufferSize)
    : _bufferSize(bufferSize)
{
    _buffer.resize(_bufferSize);

    if ((_epollFileno = epoll_create1(EPOLL_CLOEXEC)) < 0)
        throw std::runtime_error("Unable to create epoll file descriptor");
}

/* Releases the dispatcher's resources */
Dispatcher::~Dispatcher()
{
    close(_epollFileno);
}

/* Subscribes the an event sink to receive the given events on a file descriptor */
void Dispatcher::subscribe(int fileno, uint32_t eventMask, Sink *sink)
{
    epoll_event event;

    event.data.ptr = sink;
    event.events   = eventMask;

    if (epoll_ctl(_epollFileno, EPOLL_CTL_ADD, fileno, &event) != 0)
        throw std::runtime_error("Unable to add file descriptor to poll");
}

/* Changes the given file descriptor's received events and event sink */
void Dispatcher::modify(int fileno, uint32_t eventMask, Sink *sink)
{
    epoll_event event;

    event.data.ptr = sink;
    event.events   = eventMask;

    if (epoll_ctl(_epollFileno, EPOLL_CTL_MOD, fileno, &event) != 0)
        throw std::runtime_error("Unable to modify file descriptor on poll");
}

/* Unsubscribes the given file descriptor's event sink from receiving events */
void Dispatcher::unsubscribe(int fileno)
{
    epoll_event event;

    event.data.ptr = NULL;
    event.events   = 0;

    if (epoll_ctl(_epollFileno, EPOLL_CTL_DEL, fileno, &event) != 0)
        throw std::runtime_error("Unable to remove file descriptor from poll");
}

/* Waits (in the given timeout) for events to occur and dispatches them */
void Dispatcher::dispatch(int timeout)
{
    _buffer.resize(_bufferSize);

    int count = epoll_wait(_epollFileno, _buffer.data(), _bufferSize, timeout);
    if (count < 0)
        throw std::runtime_error("Unable to wait for events to occur");

    _buffer.resize(static_cast<size_t>(static_cast<unsigned int>(count)));

    for (EventBuffer::iterator event = _buffer.begin(); event != _buffer.end(); event++)
    {
        Sink *sink = static_cast<Sink *>(event->data.ptr);
        try
        {
            sink->handleEvents(event->events);
        } catch (const std::exception &exception)
        {
            sink->handleException(exception.what());
        } catch (...)
        {
            sink->handleException("Thrown type is not derived from std::exception");
        }
    }
}

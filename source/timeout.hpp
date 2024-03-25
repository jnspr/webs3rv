#ifndef TIMEOUT_hpp
#define TIMEOUT_hpp

#include <stdint.h>

/* Default timeout duration */
#define DEFAULT_TIMEOUT_MS 5000

class Timeout
{
public:
    /* Constructs a timeout starting from the current time */
    explicit Timeout(uint64_t duration = DEFAULT_TIMEOUT_MS);

    /* Resets the timeout's starting time to the current time */
    void reset();

    /* Gets whether the timeout has expired or not */
    bool isExpired();

    /* Stops the timeout so it can never expire */
    void stop();

    /* Gets whether the timeout was stopped or not */
    inline bool isStopped()
    {
        return _isStopped;
    }
private:
    uint64_t _duration;
    uint64_t _startTime;
    bool     _isStopped;

    /* Queries the current time from the operating system */
    static uint64_t getCurrentTime();
};

#endif // TIMEOUT_hpp

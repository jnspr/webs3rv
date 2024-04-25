#pragma once

class SignalManager
{
public:
    /** Registers various signal handlers */
    SignalManager();

    /** Gets if the application should quit */
    static inline bool shouldQuit()
    {
        return _instance._shouldQuit;
    }
private:
    static SignalManager _instance;
    volatile bool        _shouldQuit;

    /** Handles various quit-type signals */
    static void handleQuitSignal(int number);

    /* Disable copy-construction and copy-assignment */
    SignalManager(const SignalManager &other);
    SignalManager &operator=(const SignalManager &other);
};

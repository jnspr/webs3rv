#include "signal_manager.hpp"

#include <csignal>
#include <stdexcept>

/** Registers various signal handlers */
SignalManager::SignalManager()
    : _shouldQuit(false)
{
    if (signal(SIGINT, handleQuitSignal) == SIG_ERR)
        throw std::runtime_error("Unable to register SIGINT");
    if (signal(SIGQUIT, handleQuitSignal) == SIG_ERR)
        throw std::runtime_error("Unable to register SIGQUIT");
    if (signal(SIGTERM, handleQuitSignal) == SIG_ERR)
        throw std::runtime_error("Unable to register SIGTERM");
}

/** Handles various quit-type signals */
void SignalManager::handleQuitSignal(int number)
{
    (void)number;
    _instance._shouldQuit = true;
}

SignalManager SignalManager::_instance;

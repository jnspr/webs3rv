#include "process.hpp"

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdexcept>
#include <sys/wait.h>

/* Starts a child process using the given constant string arrays
   The arrays must be NULL-terminated, see `man execve(2)` */
Process::Process(const char **argArray, const char **envArray)
{
    startChild(argArray, envArray);
}

/* Starts a child process using the given dynamic string vectors */
Process::Process(const std::vector<std::string> &argVec, const std::vector<std::string> &envVec)
{
    std::vector<const char *> argvVector = toCharPointers(argVec);
    std::vector<const char *> envpVector = toCharPointers(envVec);
    startChild(argvVector.data(), envpVector.data());
}

/* Kills the child process and closes the socket */
Process::~Process()
{
    closeInput();
    closeOutput();
    if (getStatus() == PROCESS_RUNNING)
        kill(_pid, SIGKILL);
}

/* Closes the child's standard input */
void Process::closeInput()
{
    if (_inputFileno < 0)
        return;
    close(_inputFileno);
    _inputFileno = -1;
}

/* Closes the child's standard output */
void Process::closeOutput()
{
    if (_outputFileno < 0)
        return;
    close(_outputFileno);
    _outputFileno = -1;
}

/* Gets the current status of the process */
ProcessStatus Process::getStatus()
{
    if (_status != PROCESS_RUNNING)
        return _status;

    int result, waitStatus;
    if ((result = waitpid(_pid, &waitStatus, WNOHANG)) < 0)
        throw std::runtime_error("Unable to query process status");

    if (result != 0)
    {
        if (WIFEXITED(waitStatus) && WEXITSTATUS(waitStatus) == 0)
            _status = PROCESS_EXIT_SUCCESS;
        else
            _status = PROCESS_EXIT_FAILURE;
    }

    return _status;
}

/* Starts a child process using the given constant string arrays */
void Process::startChild(const char **argArray, const char **envArray)
{
    // Set up pipes for communication with the child
    Pipe inputPipe, outputPipe;
    setupPipeIO(inputPipe, outputPipe);

    // Fork the process and clean up on failure
    if ((_pid = fork()) < 0)
    {
        close(inputPipe.readFileno);
        close(inputPipe.writeFileno);
        close(outputPipe.readFileno);
        close(outputPipe.writeFileno);
        throw std::runtime_error("Unable to fork process");
    }

    if (_pid == 0)
    {
        // Close the parent-owned pipe FDs
        close(inputPipe.writeFileno);
        close(outputPipe.readFileno);

        // Only execute the process when both dup2() calls succeeded
        if (dup2(inputPipe.readFileno, STDIN_FILENO) >= 0 &&
            dup2(outputPipe.writeFileno, STDOUT_FILENO) >= 0)
        {
            execve(argArray[0], (char *const *)argArray, (char *const *)envArray);
        }

        // If execve() ever returns or dup2() fails, exit the process immediately
        std::exit(255);
    }

    // Close the child-owned pipes and save the parent-owned pipes
    close(inputPipe.readFileno);
    close(outputPipe.writeFileno);
    _inputFileno = inputPipe.writeFileno;
    _outputFileno = outputPipe.readFileno;

    // Initialize the status
    _status = PROCESS_RUNNING;
}

/* Converts the given vector of C++ strings into a NULL-terminated vector of C strings */
std::vector<const char *> Process::toCharPointers(const std::vector<std::string> &inputVec)
{
    std::vector<const char *> outputVector;
    outputVector.reserve(inputVec.size());

    for (size_t index = 0; index < inputVec.size(); index++)
        outputVector.push_back(inputVec[index].c_str());

    outputVector.push_back(NULL);
    return outputVector;
}

/* Sets up pipes for communication with the child process */
void Process::setupPipeIO(Pipe &inputPipe, Pipe &outputPipe)
{
    int descriptors[2];

    if (pipe(descriptors) != 0)
        throw std::runtime_error("Unable to create input pipe");

    inputPipe.readFileno = descriptors[0];
    inputPipe.writeFileno = descriptors[1];

    if (pipe(descriptors) != 0)
    {
        close(inputPipe.readFileno);
        close(inputPipe.writeFileno);
        throw std::runtime_error("Unable to create output pipe");
    }

    outputPipe.readFileno = descriptors[0];
    outputPipe.writeFileno = descriptors[1];
}

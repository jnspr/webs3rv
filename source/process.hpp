#ifndef PROCESS_hpp
#define PROCESS_hpp

#include <string>
#include <vector>

enum ProcessStatus
{
    PROCESS_RUNNING,
    PROCESS_EXIT_SUCCESS,
    PROCESS_EXIT_FAILURE
};

class Process
{
public:
    /* Starts a child process using the given constant string arrays
       The arrays must be NULL-terminated, see `man execve(2)` */
    Process(const char **argArray, const char **envArray);

    /* Starts a child process using the given dynamic string vectors */
    Process(const std::vector<std::string> &argVec, const std::vector<std::string> &envVec);

    /* Kills the child process and closes the socket */
    ~Process();

    /* Closes the child's standard input */
    void closeInput();

    /* Closes the child's standard output */
    void closeOutput();

    /* Gets the current status of the process */
    ProcessStatus getStatus();

    /* Gets the child's process ID */
    inline int getPid()
    {
        return _pid;
    }

    /* Gets the file descriptor for writing into the child's standard input */
    inline int getInputFileno()
    {
        return _inputFileno;
    }

    /* Gets the file descriptor for reading from the child's standard output */
    inline int getOutputFileno()
    {
        return _outputFileno;
    }
private:
    /* POD struct holding the file descriptors for a pipe */
    struct Pipe
    {
        int readFileno;
        int writeFileno;
    };

    int           _pid;
    ProcessStatus _status;
    int           _inputFileno;
    int           _outputFileno;

    /* Starts a child process using the given constant string arrays */
    void startChild(const char **argArray, const char **envArray);

    /* Converts the given vector of C++ strings into a NULL-terminated vector of C strings */
    static std::vector<const char *> toCharPointers(const std::vector<std::string> &inputVec);

    /* Sets up pipes for communication with the child process */
    void setupPipeIO(Pipe &inputPipe, Pipe &outputPipe);

    /* Disable copy-construction and copy-assignment */
    Process(const Process &other);
    Process &operator=(const Process &other);
};

#endif // PROCESS_hpp

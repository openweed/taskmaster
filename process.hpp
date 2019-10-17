#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <signal.h>

#include <string>
#include <vector>

enum process_state{
    PROCESS_STATE_STOPEED,
    PROCESS_STATE_PAUSED,
    PROCESS_STATE_RUNNING
};

class process
{
public:
    process(const std::string program_path, const std::vector<const std::string> args, bool run = false);
    ~process();
    int start();
    int stop(int signal = SIGINT);
    int signal(int signal);
    void set_args(const std::vector<const std::string> args);
protected:
private:
    const std::string path;
    char **argv;
    char **envp;
    process_state state;
    int exit_code;
    void free_args();
};

#endif // PROCESS_HPP

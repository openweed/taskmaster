#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

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
    process(const std::string &program_path,
            const std::vector<std::string> &args = {},
            bool run = false);
    ~process();
    pid_t start();
    int stop(int signal = SIGINT);
    int signal(int signal);
    void set_args(const std::vector<std::string> &args);
    void set_dir(const std::string work_dir) {dir = work_dir;}
protected:
private:
    const std::string path;
    char **argv;
    char **envp;
    std::string dir;
    pid_t pid;
    process_state state;
    int exit_code;
    void free_argv();
};

#endif // PROCESS_HPP

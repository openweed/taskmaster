#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <fcntl.h>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <memory>

namespace proc{

class process
{
public:
    enum class process_state : int {
        ERROR,
        DID_NOT_START,
        RUNNING,
        STOPPED,
        SIGNALED,
        EXITED,
        TERMINATED
    };
    process(const std::string &program_path);

    process() = delete;
    process(const process &other);
    process& operator=(const process &other);
    process(process &&other) noexcept;
    process& operator=(process &&other) noexcept;
    ~process() {stop(SIGKILL);}

    void set_args(const std::vector<std::string> &arguments = {});
    void set_envs(const std::vector<std::string> &variables = {});
    void set_workdir(const std::string &dir) {workdir = dir;}
    void set_redirection(const std::string &stdin_file_path,
                         const std::string &stdout_file_path,
                         const std::string &stderr_file_path);
    void set_stoptime(time_t time) {stoptime = time;}
    void set_umask(mode_t mode) {mask = mode;}

    pid_t start();
    void stop(int sig = SIGTERM) noexcept;
    bool update(bool wait = false);
    int signal(int sig);

    process_state get_state() {return state;}
    int get_pid() {return pid;}
    int get_exitcode() {return exitstatus;}
    int get_termsignal() {return termsig;}
    bool is_exited() {return state == process_state::EXITED;}
    bool is_signaled() {return state == process_state::SIGNALED;}
    bool is_exist() {return (state == process_state::RUNNING) ||
                     (state == process_state::STOPPED);}
    bool is_error() {return state == process_state::ERROR;}

protected:
private:
    // Process config
    std::string bin;                        // Binary file path, is used for execve()
    std::vector<std::string> args;          // Programm arguments
    std::vector<std::string> envs;          // Programm environment
    std::string workdir = "/";              // Working directory
    std::string stdin_file = "/dev/null";   // stdin file
    std::string stdout_file = "/dev/null";  // stdout file
    std::string stderr_file = "/dev/null";  // stderr file
    time_t stoptime = 10;                   // Maximum process stop time until SIGKILL is received
    mode_t mask = S_IWGRP | S_IWOTH;

    // Process status
    std::vector<const char *> argv;          // Pointers to c_str in args, is used for execve()
    std::vector<const char *> envp;          // Pointers to c_str in envs, is used for execve()
    process_state state = process_state::DID_NOT_START;
    pid_t pid = 0;
    int exitstatus = 0;                      // Process exit status, it makes sense if the process exited
    int stopsig = 0;                         // Process exit status, it makes sense if the process stopped
    int termsig = 0;                         // Process exit status, it makes sense if the process exited by signal

    void apply_redir();                       // Applies redirection, used after fork()
    void set_argv();
    void set_envp();
};

} // namespace proc

#endif // PROCESS_HPP

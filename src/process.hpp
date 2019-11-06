#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <csignal>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <memory>

namespace proc{

const time_t DEFAULT_STOPTIME = 10;

class process
{
public:
    process(std::string program_path);

    process() = delete;
    process(const process &other);
    process& operator=(const process &other);
    process(process &&other) noexcept;
    process& operator=(process &&other) noexcept;
    ~process() {stop(SIGKILL);}

    pid_t start();
    void stop(int sig = SIGTERM);
    bool update(bool wait = false);
    int signal(int sig);
    bool is_exited();
    bool is_running();
    bool is_launched();
    int get_state() {return state;}
    int get_pid() {return pid;}
    void set_args(const std::vector<std::string> &arguments = {});
    void set_envs(const std::vector<std::string> &variables = {});
    void set_workdir(const std::string &dir) {workdir = dir;}
    void set_redirection(const std::string &stdin_file_path,
                         const std::string &stdout_file_path,
                         const std::string &stderr_file_path);
    void set_stoptime(time_t time) {stoptime = time;}

protected:
private:
    std::string bin;                     // Binary file path, is used for execve()
    std::vector<const char *> argv;      // Pointers to c_str in args, is used for execve()
    std::vector<const char *> envp;      // Pointers to c_str in envs, is used for execve()
    std::vector<std::string> args;       // Programm arguments
    std::vector<std::string> envs;       // Programm environment
    std::string workdir;                 // Working directory
    std::string stdin_file;              // stdin file
    std::string stdout_file;             // stdout file
    std::string stderr_file;             // stderr file
    time_t stoptime;                     // Maximum process stop time until SIGKILL is received
    enum process_status {
        DID_NOT_START,
        RUNNING,
        STOPPED,
        SIGNALED,
        EXITED,
        TERMINATED
    } state;                             // Process state
    pid_t pid;                           // Process pid, it makes sense if the process is running
    int exitstatus;                      // Process exit status, it makes sense if the process exited
    int stopsig;                         // Process exit status, it makes sense if the process stopped
    int termsig;                         // Process exit status, it makes sense if the process exited by signal

    void apply_redir();                  // Applies redirection, used after fork()
    void basic_init();                   // Initialization of some variables, for example argv
};

} // namespace proc

#endif // PROCESS_HPP

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <memory>

enum process_status {
    PROCESS_DID_NOT_START,
    PROCESS_RUNNING,
    PROCESS_STOPPED,
    PROCESS_SIGNALED,
    PROCESS_EXITED
};

class process
{
public:
    process() = delete;
    process(const process &other) = delete;
    process operator=(const process &other) = delete;
    process(process &&other);
    process operator=(process &&other) = delete;

    process(const std::string &program_path);

    pid_t start();
    void stop(int sig = SIGTERM);
    bool update();
    int signal(int sig);
    bool is_exited() {return (status == PROCESS_SIGNALED) || status == PROCESS_EXITED;}
    bool is_running() {return (status == PROCESS_RUNNING);}
    bool is_launched() {return (status == PROCESS_RUNNING) || (status == PROCESS_STOPPED);}
    process_status get_status() {return status;}
    void set_args(const std::vector<std::string> &arguments = {});
    void set_envs(const std::vector<std::string> &variables = {});
    void set_workdir(const std::string work_dir) {dir = work_dir;}
    void set_redirection(const std::string &stdin_file_path,
                         const std::string &stdout_file_path,
                         const std::string &stderr_file_path);

protected:
private:
    const std::string bin;
    std::vector<const char *> argv;
    std::vector<const char *> envp;
    std::vector<std::string> args;
    std::vector<std::string> envs;
    std::string dir;
    std::string stdin_file;
    std::string stdout_file;
    std::string stderr_file;
    pid_t pid;
    process_status status;
    int exitstatus;
    int stopsig;
    int termsig;

    void apply_redir();
};

#endif // PROCESS_HPP

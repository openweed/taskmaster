#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>

#include "process.hpp"

extern char **environ;

//
#include <iostream>
///

using namespace std;

process::process(const string &program_path) :
    bin(program_path), dir("/"), stdin_file("/dev/null"), stdout_file("/dev/null"), stderr_file("/dev/null")
{
    status = PROCESS_DID_NOT_START;
    exitstatus = 0;
    termsig = -1;
    stopsig = -1;
    set_args();
    set_envs();
}

pid_t process::start()
{
    if (access(bin.c_str(), X_OK)) return -errno;
    if ((pid = fork())) {
        status = PROCESS_RUNNING;
        return pid;
    }
    if (pid == -1) {
        if (errno == EAGAIN) return start();
        return -errno;
    }
    apply_redir();
    // XXX maybe change pgroup
    chdir(dir.c_str());
    execve(bin.c_str(), const_cast<char **>(argv.data()), const_cast<char **>(envp.data()));
    exit(1);
}

void process::stop(int sig)
{
    signal(sig);
    update();
}

// Возвращает true если что то изменилось
bool process::update()
{
    if (!is_launched()) return false;
    int status;
    if (!waitpid(pid, &status, WUNTRACED | WNOHANG))
        return false;
    if (WIFSTOPPED(status)) {
        stopsig = WSTOPSIG(status);
        status = PROCESS_STOPPED;
    } else if (WIFSIGNALED(status)) {
        termsig = WTERMSIG(status);
        status = PROCESS_SIGNALED;
    } else if (WIFEXITED(status)) {
        exitstatus = WEXITSTATUS(status);
        status = PROCESS_EXITED;
    }
    return true;
}

int process::signal(int sig)
{
    if (!is_launched()) {
        errno = ESRCH;
        return ESRCH;
    }
    return kill(pid, sig);
}

void process::set_args(const std::vector<string> &arguments)
{
    args = arguments;
    argv.clear();
    argv.resize(args.size() + 2ul);
    size_t i = 0;
    argv[i++] = bin.c_str();
    for (const auto &arg: args)
        argv[i++] = arg.c_str();
    argv[i] = nullptr;
}
void process::set_envs(const std::vector<std::string> &variables = {})
{
    envs = variables;
    envp.clear();
    size_t env_count = 0;
    while (environ[env_count]) env_count++;
    envp.resize(env_count + envs.size() + 1ul);
    size_t i = 0;
    while (environ[i]) {
        envp[i] = environ[i];
        i++;
    }
    // XXX need constant environ array
}

void process::set_redirection(const std::string &stdin_file_path, const std::string &stdout_file_path,
                     const std::string &stderr_file_path)
{
    stdin_file = stdin_file_path;
    stdout_file = stdout_file_path;
    stderr_file = stderr_file_path;
}

/*
 * Private
 */

void process::apply_redir()
{
    int stdin_fd = open(stdin_file.c_str(), O_RDONLY | O_CLOEXEC);
    int stdout_fd = open(stdout_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    int stderr_fd = open(stderr_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    int devnull_fd = open("/dev/null", O_RDWR | O_CREAT |  O_CLOEXEC, 0644);
    if (dup2(stdin_fd, STDIN_FILENO) == -1 && dup2(devnull_fd, STDIN_FILENO) == -1)
        close(STDIN_FILENO);
    if (dup2(stdout_fd, STDOUT_FILENO) == -1 && dup2(devnull_fd, STDOUT_FILENO) == -1)
        close(STDOUT_FILENO);
    if (dup2(stderr_fd, STDERR_FILENO) == -1 && dup2(devnull_fd, STDERR_FILENO) == -1)
        close(STDERR_FILENO);
}

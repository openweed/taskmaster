#include <utility>

#include <fcntl.h>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>

#include "process.hpp"

//
#include <iostream>
using namespace std;
///

using namespace proc;

void process::basic_init()
{
    state = DID_NOT_START;
    exitstatus = termsig = stopsig = 0;
    set_args(args);
    set_envs(envs);
}

process::process(string program_path) :
    bin(std::move(program_path)), workdir("/")
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    basic_init();
}

process::process(const process &other) :
    bin(other.bin), args(other.args), envs(other.envs), workdir(other.workdir),
    stdin_file(other.stdin_file), stdout_file(other.stdout_file), stderr_file(other.stderr_file)
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    basic_init();
}

process& process::operator=(const process &other)
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    if (&other == this) return *this;
    stop(true); // Stop current process if running
    bin = other.bin;
    args = other.args;
    envs = other.envs;
    workdir = other.workdir;
    stdin_file = other.stdin_file;
    stdout_file = other.stdout_file;
    stderr_file = other.stderr_file;
    basic_init();
    return *this;
}

process::process(process &&other) noexcept:
    bin(move(other.bin)), args(move(other.args)), envs(move(other.envs)),
    workdir(move(other.workdir)), stdin_file(move(other.stdin_file)),
    stdout_file(move(other.stdout_file)), stderr_file(move(other.stderr_file)),
    state(other.state), pid(other.pid), exitstatus(other.exitstatus),
    stopsig(other.stopsig), termsig(other.termsig)
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    set_args(args);
    set_envs(envs);

    other.state = DID_NOT_START; // Destructor 'other' must not stop the process
}

process& process::operator=(process &&other) noexcept
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    stop(true); // Stop current process if running
    bin = move(other.bin);
    args = move(other.args);
    envs = move(other.envs);
    workdir = move(other.workdir);
    stdin_file = move(other.stdin_file);
    stdout_file = move(other.stdout_file);
    stderr_file = move(other.stderr_file);
    pid = other.pid;
    state = other.state;
    exitstatus = other.exitstatus;
    stopsig = other.stopsig;
    termsig = other.termsig;

    set_args(args);
    set_envs(envs);
    update();

    return *this;
}

pid_t process::start()
{
    update();
    if (is_launched()) return pid;
    if (access(bin.c_str(), X_OK)) return -errno;

    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    if ((pid = fork())) {
        state = process::RUNNING;
        return pid;
    }
    if (pid == -1) {
        if (errno == EAGAIN) return start();
        return -errno;
    }
    apply_redir();
    chdir(workdir.c_str());
    execve(bin.c_str(), const_cast<char **>(argv.data()), const_cast<char **>(envp.data()));
    exit(1);
}

void process::stop(bool hard)
{
    // If the process is not launched, then return
    if (!is_launched()) return;
    signal(hard ? SIGKILL : SIGTERM);
    update();
}

// Возвращает true если что то изменилось
bool process::update()
{
    if (!is_launched()) return false;
    int status;
    pid_t res;
    if (!(res = waitpid(pid, &status, WUNTRACED | WNOHANG)))
        return false;
    if (res < 0) {
        exitstatus = 0;
        state = process::EXITED;
    } else if (WIFSTOPPED(status)) {
        stopsig = WSTOPSIG(status);
        state = process::STOPPED;
    } else if (WIFSIGNALED(status)) {
        termsig = WTERMSIG(status);
        state = process::SIGNALED;
    } else if (WIFEXITED(status)) {
        exitstatus = WEXITSTATUS(status);
        state = process::EXITED;
    } else {
        state = process::DID_NOT_START;
    }
    cout << "update state: " << state << endl;
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
    argv.resize(args.size() + 2U);
    size_t i = 0;
    argv[i++] = bin.c_str();
    for (const auto &arg: args)
        argv[i++] = arg.c_str();
    argv[i] = nullptr;
}

void process::set_envs(const std::vector<std::string> &variables)
{
    envs = variables;
    envp.clear();
    size_t i = 0;
    for (auto &var : envs)
        envp[i++] = var.c_str();
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
   if (!freopen(stdin_file.c_str(), "r", stdin)) freopen("/dev/null", "r", stdin);
   if (!freopen(stdout_file.c_str(), "a", stdout)) freopen("/dev/null", "w", stdout);
   if (!freopen(stderr_file.c_str(), "a", stderr)) freopen("/dev/null", "w", stderr);
}

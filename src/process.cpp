#include <iostream>
#include <utility>
#include <cstring>
#include <functional>
#include <thread>

#include <fcntl.h>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>


#include "process.hpp"

using namespace std;
using namespace proc;

process::process(const string &program_path) : bin(program_path)
{
    set_argv();
    set_envp();
}

process::process(const process &other) :
    bin(other.bin), args(other.args), envs(other.envs), workdir(other.workdir),
    stdin_file(other.stdin_file), stdout_file(other.stdout_file),
    stderr_file(other.stderr_file)
{
    set_argv();
    set_envp();
}

process& process::operator=(const process &other)
{
    if (&other == this) return *this;
    // Stop current process if running
    stop(SIGKILL);
    bin = other.bin;
    args = other.args;
    envs = other.envs;
    workdir = other.workdir;
    stdin_file = other.stdin_file;
    stdout_file = other.stdout_file;
    stderr_file = other.stderr_file;
    stoptime = other.stoptime;
    mask = other.mask;
    set_argv();
    set_envp();
    return *this;
}

process::process(process &&other) noexcept:
    bin(move(other.bin)), args(move(other.args)), envs(move(other.envs)),
    workdir(move(other.workdir)), stdin_file(move(other.stdin_file)),
    stdout_file(move(other.stdout_file)), stderr_file(move(other.stderr_file)),
    stoptime(other.stoptime), mask(other.mask), state(other.state),
    pid(other.pid), exitstatus(other.exitstatus), stopsig(other.stopsig),
    termsig(other.termsig)
{
    set_argv();
    set_envp();
    // Destructor 'other' must not stop the process
    other.state = process_state::EXITED;
}

process& process::operator=(process &&other) noexcept
{
    // Stop current process if running
    stop(SIGKILL);
    bin = move(other.bin);
    args = move(other.args);
    envs = move(other.envs);
    workdir = move(other.workdir);
    stdin_file = move(other.stdin_file);
    stdout_file = move(other.stdout_file);
    stderr_file = move(other.stderr_file);
    stoptime = other.stoptime;
    pid = other.pid;
    state = other.state;
    exitstatus = other.exitstatus;
    stopsig = other.stopsig;
    termsig = other.termsig;
    set_argv();
    set_envp();
    return *this;
}

pid_t process::start()
{
    if (is_exist()) return pid;
    if (access(bin.c_str(), X_OK)) {
        state = process_state::ERROR;
        throw runtime_error(string("failed to start the process: ") +
                            strerror(errno));
    }
    if ((pid = fork()) > 0) {
        state = process_state::RUNNING;
        return pid;
    }
    if (pid == -1) {
        if (errno == EAGAIN) {
            return start();
        }
        throw runtime_error(string("failed to start the process") +
                            strerror(errno));
    }
    setpgrp();
    apply_redir();
    umask(mask);
    chdir(workdir.c_str());
    execve(bin.c_str(), const_cast<char **>(argv.data()), const_cast<char **>(envp.data()));
    exit(1);
}

void process::stop(int sig) noexcept
{
    // If the process is not launched, then return
    if (!is_exist()) return;
    signal(sig);
    state = process_state::TERMINATED;
    termsig = sig;
    auto stop_pid = pid;
    pid = 0;
    if (sig == SIGKILL) {
        waitpid(stop_pid, nullptr, 0);
        return;
    }
    std::function<void(pid_t, time_t)> termfunc = [](pid_t pid, time_t stoptime) {
        sleep(stoptime);
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
    };
    std::thread termthread(termfunc, stop_pid, stoptime);
    termthread.detach();
}

// Returns true if state changed
bool process::update(bool wait)
{
    if (!is_exist()) return false;
    int status;
    pid_t res;
    if (!(res = waitpid(pid, &status, wait ? 0 : (WUNTRACED | WNOHANG))))
        return false;
    if (res < 0) {
        exitstatus = 0;
        state = process_state::EXITED;
    } else if (WIFSTOPPED(status)) {
        stopsig = WSTOPSIG(status);
        state = process_state::STOPPED;
    } else if (WIFSIGNALED(status)) {
        termsig = WTERMSIG(status);
        state = process_state::SIGNALED;
    } else if (WIFEXITED(status)) {
        exitstatus = WEXITSTATUS(status);
        state = process_state::EXITED;
    } else {
        state = process_state::TERMINATED;
    }
    return true;
}

int process::signal(int sig)
{
    if (!is_exist()) {
        errno = ESRCH;
        return -1;
    }
    return kill(pid, sig);
}


void process::set_args(const std::vector<string> &arguments)
{
    args = arguments;
    set_argv();
}

void process::set_envs(const std::vector<std::string> &variables)
{
    envs = variables;
    set_envp();
}

void process::set_redirection(const std::string &stdin_file_path,
                              const std::string &stdout_file_path,
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

void process::set_argv()
{
    argv.clear();
    argv.push_back(bin.c_str());
    for (const auto &arg: args) argv.push_back(arg.c_str());
    argv.push_back(nullptr);
}

void process::set_envp()
{
    envp.clear();
    for (auto &var : envs) envp.push_back(var.c_str());
    envp.push_back(nullptr);
}

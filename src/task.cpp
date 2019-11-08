#include <algorithm>

#include "unistd.h"
#include "sys/types.h"

#include "task.hpp"

using namespace tasks;

//
#include <iostream>
using namespace std;

task_config::task_config() : numprocs(DEFAULT_NUMPROC), mask(DEFAULT_MASK),
    directory(DEFAULT_WORKDIR), autostart(DEFAULT_AUTOSTART),
    autorestart(DEFAULT_AUTORESTART), exitcodes(DEFAULT_EXIT_CODES),
    startretries(DEFAULT_STARTRETRIES), startsecs(DEFAULT_TIMETOSTART),
    stopsignal(DEFAULT_STOPSIG), stopsecs(DEFAULT_STOPTIME), stdin_file(DEFAULT_REDIR),
    stdout_file(DEFAULT_REDIR), stderr_file(DEFAULT_REDIR)
{}

task_status::task_status() : state(STOPPED), starttries(0), starttime(0)
{}

task::task(struct task_config &configuration) : config(configuration)
{
    processes.resize(config.numprocs, make_pair(proc::process(config.bin), task_status()));
    for (auto &proc: processes) {
        proc.first.set_args(config.args);
        proc.first.set_envs(config.envs);
        proc.first.set_workdir(config.directory);
        proc.first.set_redirection(config.stdin_file, config.stdout_file, config.stderr_file);
        proc.first.set_stoptime(config.stopsecs);
        proc.first.set_umask(config.mask);
    }
    if (config.autostart) start();
}

void task::start()
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    for (size_t i = 0; i < config.numprocs; ++i)
        start(i);
}

void task::start(size_t index)
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    // XXX check?
    if (index >= processes.size()) return;
    update(index);
    processes[index].first.start();
    processes[index].second.state = task_status::STARTING;
    processes[index].second.starttime = time(nullptr);
    processes[index].second.starttries++;
}

void task::stop()
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    for (size_t i = 0; i < config.numprocs; ++i)
        stop(i);
}

void task::stop(size_t index)
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    if (index >= processes.size()) return;
    processes[index].first.stop(config.stopsignal);
    processes[index].second.state = task_status::STOPPED;
    processes[index].second.starttries = 0;
}

void task::restart()
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    stop();
    start();
}

void task::restart(size_t index)
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    if (index >= processes.size()) return;
    stop(index);
    start(index);
}

bool task::update()
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    bool res = false;
    for (size_t i = 0; i < config.numprocs; ++i)
        res |= update(i);
    return res;
}

bool task::update(size_t index)
{
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    if (index >= processes.size()) return false;
    auto state = processes[index].second.state;
    if (state != task_status::STARTING && state != task_status::RUNNING) return false;

    if (time(nullptr) - processes[index].second.starttime >= config.startsecs)
        processes[index].second.state = task_status::RUNNING;

    if (processes[index].first.is_exist())
        return processes[index].second.state != state;

    // Process died
    if (state == task_status::STARTING) {
        if (processes[index].second.starttries < config.startretries) {
            start(index);
        } else {
            processes[index].second.state = task_status::FATAL;
            processes[index].second.starttries = 0;
        }
    } else if (state == task_status::RUNNING) {
        if (config.autorestart == task_config::TRUE) {
            start(index);
        } else if (config.autorestart == task_config::UNEXPECTED && !is_exited_normally(index)) {
            start(index);
        } else {
            processes[index].second.state = task_status::EXITED;
            processes[index].second.starttries = 0;
        }
    }
    return processes[index].second.state != state;
}

// Returns true if the process completed successfully or was stopped by the user
bool task::is_exited_normally(size_t index)
{
    if (!processes[index].first.is_exited())
        return false;
    // XXX
//    if (processes[index].first.is_signaled() &&
//        processes[index].first.get_termsignal() == config.stopsignal)
//        return true;
    return std::find(config.exitcodes.begin(), config.exitcodes.end(),
                     processes[index].first.get_exitcode()) != config.exitcodes.cend();
}

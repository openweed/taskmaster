#include "task.hpp"

using namespace tasks;

//
#include <iostream>
using namespace std;

task_config::task_config() : numproc(DEFAULT_NUMPROC), mask(DEFAULT_MASK),
    workdir(DEFAULT_WORKDIR), autostart(DEFAULT_AUTOSTART),
    autorestart(DEFAULT_AUTORESTART), exit_codes(DEFAULT_EXIT_CODES),
    startretries(DEFAULT_STARTRETRIES), starttime(DEFAULT_STARTTIME),
    stopsig(DEFAULT_STOPSIG), stoptime(DEFAULT_STOPTIME), stdin_file(DEFAULT_REDIR),
    stdout_file(DEFAULT_REDIR), stderr_file(DEFAULT_REDIR)
{
}

task::task(task_config &config) : state(STOPPED), config(config)
{
    pgid = 0;
    procs.resize(config.numproc, config.bin);
    for (auto &proc: procs) {
        proc.set_args(config.args);
        proc.set_envs(config.envs);
        proc.set_workdir(config.workdir);
        proc.set_redirection(config.stdin_file, config.stdout_file, config.stderr_file);
        if (config.autostart) proc.start();
    }
}

void task::start()
{
    if (state == RUNNING) return;
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    for (auto &proc: procs) {
        proc.start();
    }
    state = RUNNING;
}

void task::stop()
{
    if (state == STOPPED) return;
    cout << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    for (auto &proc: procs) {
        proc.stop();
    }
    state = STOPPED;
}

void task::restart()
{
    stop();
    start();
}

bool task::update()
{
    bool res = false;
    for (auto &proc: procs) {
        res |= proc.update();
    }
    return res;
}

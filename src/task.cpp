#include "task.hpp"

task_config::task_config() : name(), bin(),
    args(), envs(), numproc(0), mask(0655), workdir("/"),
    autostart(false), autorestart(false), exit_codes({0}),
    startretries(0), starttime(5), stopsig(SIGTERM), stoptime(10),
    stdin_file("/dev/null"), stdout_file("/dev/null"), stderr_file("/dev/null")
{
}

task::task(task_config_t &config) : config(config)
{
    //procs.resize(config.numproc, config.bin);
    //procs.push_back(std::string("/bin"));
//    for (auto &proc: procs) {
//        proc.set_args(config.args);
//        proc.set_workdir(config.workdir);
//        proc.set_redirection(config.stdin_file, config.stdout_file, config.stderr_file);
//    }
}

void task::start()
{
}

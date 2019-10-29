#ifndef TASK_HPP
#define TASK_HPP

#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <string>

#include "process.hpp"
#include "task.hpp"

// XXX need copy and move constructors
typedef struct task_config
{
    task_config();
    std::string name;
    std::string bin;
    std::vector<std::string> args;
    std::vector<std::string> envs;
    size_t numproc;
    mode_t mask;
    std::string workdir;
    bool autostart;
    bool autorestart;
    std::vector<int> exit_codes;
    size_t startretries;
    time_t starttime;
    int stopsig;
    time_t stoptime;
    std::string stdin_file;
    std::string stdout_file;
    std::string stderr_file;
} task_config_t;

class task
{
public:
    task(task_config_t &config);
    void start();
    void stop();
    int restart();
private:
    task_config_t config;
    std::vector<process> procs;
    pid_t pgid;
};

#endif // TASK_HPP

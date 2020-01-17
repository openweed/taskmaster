#ifndef TASK_HPP
#define TASK_HPP

#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <string>

#include "process.hpp"

struct task_config;

void print_config(const task_config &tconf, std::ostream &stream);
std::vector<task_config> tconfs_from_yaml(const std::string &file);

struct task_config
{
    task_config() = default;
    std::string name;
    std::string bin;
    std::vector<std::string> args;
    std::vector<std::string> envs;
    size_t numprocs = 1;
    mode_t mask = 022;
    std::string workdir = "/";
    bool autostart = false;
    enum {
        FALSE,
        UNEXPECTED,
        TRUE
    } autorestart = FALSE;
    std::vector<int> exitcodes = {0};
    size_t startretries = 0;
    time_t startsecs = 5;
    int stopsignal = SIGTERM;
    time_t stopsecs = 10;
    std::string stdin_file = "/dev/null";
    std::string stdout_file = "/dev/null";
    std::string stderr_file = "/dev/null";
};

struct task_status
{
    task_status() = default;
    enum {
        STOPPED,
        STARTING,
        RUNNING,
        EXITED,
        FATAL,
        ERROR,
        UNKNOWN
    } state = STOPPED;
    size_t starttries = 0;
    time_t starttime = 0;
};

class task : private std::vector<proc::process>
{
public:
    task(const task_config &tconf);
    void start();
    void stop();
    void restart();
    std::string status();
    void update();
private:
    void exec();
    void kill(int signal = SIGKILL);
    bool is_exited_normally(proc::process &p);
    struct task_config config;
    struct task_status state;
};

#endif // TASK_HPP

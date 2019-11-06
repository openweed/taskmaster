#ifndef TASK_HPP
#define TASK_HPP

#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <string>

#include "process.hpp"

// XXX need copy and move constructors
namespace tasks {

const int              DEFAULT_NUMPROC = 1;
const int              DEFAULT_MASK = 0655;
const std::string      DEFAULT_WORKDIR = std::string("/");
const bool             DEFAULT_AUTOSTART = false;
const bool             DEFAULT_AUTORESTART = false;
const std::vector<int> DEFAULT_EXIT_CODES = std::vector<int>({0});
const size_t           DEFAULT_STARTRETRIES = 0;
const time_t           DEFAULT_STARTTIME = 5;
const int              DEFAULT_STOPSIG = SIGTERM;
const time_t           DEFAULT_STOPTIME = 10;
const std::string      DEFAULT_REDIR = std::string("/dev/null");

struct task_config
{
public:
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
};

class task
{
public:
    task(task_config &config);
    void start();
    void stop();
    void restart();
    bool update();
private:
    enum {
        STOPPED,
        RUNNING
    } state;
    task_config config;
    std::vector<proc::process> procs;
    pid_t pgid;
};

} // namespace tasks

#endif // TASK_HPP

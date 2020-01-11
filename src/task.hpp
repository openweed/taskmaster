#ifndef TASK_HPP
#define TASK_HPP

#include <sys/types.h>
#include <sys/stat.h>

#include <vector>
#include <string>

#include "process.hpp"

// XXX need copy and move constructors
//namespace tasks {
struct task_config;

void print_config(const task_config &tconf, std::ostream &stream);
std::vector<task_config> config_from_yaml(const std::string &file);

struct task_config
{
    task_config();
    std::string name;
    std::string bin;
    std::vector<std::string> args;
    std::vector<std::string> envs;
    size_t numprocs;
    mode_t mask;
    std::string workdir;
    bool autostart;
    enum {
        FALSE,
        UNEXPECTED,
        TRUE
    } autorestart;
    std::vector<int> exitcodes;
    size_t startretries;
    time_t startsecs;
    int stopsignal;
    time_t stopsecs;
    std::string stdin_file;
    std::string stdout_file;
    std::string stderr_file;
};

const int              DEFAULT_NUMPROC = 1;
const int              DEFAULT_MASK = 022;
const std::string      DEFAULT_WORKDIR = std::string("/");
const bool             DEFAULT_AUTOSTART = false;
const auto             DEFAULT_AUTORESTART = task_config::FALSE;
const std::vector<int> DEFAULT_EXIT_CODES = std::vector<int>({0});
const size_t           DEFAULT_STARTRETRIES = 0;
const time_t           DEFAULT_TIMETOSTART = 5;
const int              DEFAULT_STOPSIG = SIGTERM;
const time_t           DEFAULT_STOPTIME = 10;
const std::string      DEFAULT_REDIR = std::string("/dev/null");

struct task_status
{
    task_status();
    enum {
        STOPPED,
        STARTING,
        RUNNING,
        EXITED,
        FATAL,
        UNKNOWN
    } state;
    size_t starttries;
    time_t starttime;
};

class task
{
public:
    task(const task_config &configuration);
    void start();
    void start(size_t index);
    void stop();
    void stop(size_t index);
    void restart();
    void restart(size_t index);
    void status();
    bool update();
    bool update(size_t index);
private:
    bool is_exited_normally(size_t index);
    struct task_config config;
    std::vector<std::pair<proc::process, struct task_status>> processes;
};

//} // namespace tasks


#endif // TASK_HPP

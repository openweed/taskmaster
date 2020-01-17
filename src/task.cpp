#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <exception>
#include <yaml-cpp/yaml.h>

#include <ctime>

#include "unistd.h"
#include "sys/types.h"

#include "task.hpp"

//using namespace tasks;

using namespace std;

static map<int, string> _states_map = {
    {task_status::STOPPED,  "stoppped"},
    {task_status::STARTING, "starting"},
    {task_status::RUNNING,  "running"},
    {task_status::EXITED,   "exited"},
    {task_status::FATAL,    "fatal"},
    {task_status::UNKNOWN,  "unknown (fatal error)"}
};

static void _config_read_prog(const YAML::Node &param, task_config &tconf);
static void _config_read_args(const YAML::Node &param, task_config &tconf);
static void _config_read_numprocs(const YAML::Node &param, task_config &tconf);
static void _config_read_umask(const YAML::Node &param, task_config &tconf);
static void _config_read_workingdir(const YAML::Node &param, task_config &tconf);
static void _config_read_autostart(const YAML::Node &param, task_config &tconf);
static void _config_read_autorestart(const YAML::Node &param, task_config &tconf);
static void _config_read_exitcodes(const YAML::Node &param, task_config &tconf);
static void _config_read_startretries(const YAML::Node &param, task_config &tconf);
static void _config_read_starttime(const YAML::Node &param, task_config &tconf);
static void _config_read_stopsignal(const YAML::Node &param, task_config &tconf);
static void _config_read_stoptime(const YAML::Node &param, task_config &tconf);
static void _config_read_stdout(const YAML::Node &param, task_config &tconf);
static void _config_read_stderr(const YAML::Node &param, task_config &tconf);
static void _config_read_env(const YAML::Node &param, task_config &tconf);


task::task(const task_config &tconf) : config(tconf)
{
    resize(config.numprocs, config.bin);
//    processes.resize(config.numprocs,
//                     make_pair(proc::process(config.bin), task_status()));
    for (auto &proc: *this) {
        proc.set_args(config.args);
        proc.set_envs(config.envs);
        proc.set_workdir(config.workdir);
        proc.set_redirection(config.stdin_file, config.stdout_file,
                                   config.stderr_file);
        proc.set_stoptime(config.stopsecs);
        proc.set_umask(config.mask);
    }
    if (config.autostart) start();
}

void task::exec()
{
    try {
        for (auto &proc: *this) proc.start();
    } catch (const exception &e) {
        for (auto &proc: *this) proc.start();
        state.state = task_status::FATAL;
        throw runtime_error(e.what());
    }
    state.starttime = time(nullptr);
    state.starttries++;
    state.state = task_status::STARTING;
}

void task::start()
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    if (state.state == task_status::UNKNOWN)
        throw runtime_error("fatal error");
    if (state.state == task_status::FATAL)
        throw runtime_error("process error");
    if (state.state == task_status::STARTING ||
        state.state == task_status::RUNNING )
        throw runtime_error("process already started");
    state.starttries = 0;
    exec();
}

//void task::start(size_t index)
//{
//    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
//    // XXX check?
//    if (index >= processes.size()) return;
//    update(index);
//    processes[index].first.start();
//    processes[index].second.state = task_status::STARTING;
//    processes[index].second.starttime = time(nullptr);
//    processes[index].second.starttries++;
//}

void task::kill(int signal)
{
    for (auto &proc: *this) proc.stop(signal);
}

void task::stop()
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    kill(config.stopsignal);
    state.state = task_status::STOPPED;
    state.starttries = 0;
    state.starttime = 0;
//    for (size_t i = 0; i < config.numprocs; ++i)
//        stop(i);
}

//void task::stop(size_t index)
//{
//    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
//    if (index >= processes.size()) return;
//    processes[index].first.stop(config.stopsignal);
//    processes[index].second.state = task_status::STOPPED;
//    processes[index].second.starttries = 0;
//}

void task::restart()
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    stop();
    start();
}
string task::status()
{
    string s(config.name + ":\n");
    s += "    state: " + _states_map[state.state] + "\n";
    if (state.state == task_status::STARTING ||
        state.state == task_status::RUNNING) {
        s += string("    starttime: ") + ctime(&state.starttime);
        s += string("    starttries: ") + to_string(state.starttries) + "\n";
    }
    return s;
}

void task::update()
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    auto s = state.state;
    if (s != task_status::STARTING && s != task_status::RUNNING) return;

    if (time(nullptr) - state.starttime >= config.startsecs)
        state.state = task_status::RUNNING;

    for (auto p : *this) {
        p.update();
        if (p.is_exist()) continue; // Process running
        // Process died
        task::kill(SIGKILL); // Kill other processes
        if (s == task_status::STARTING) { // FATAL or restart
            if (state.starttries < config.startretries) {
                task::exec(); // Restart processes
            } else {
                state.state = task_status::FATAL; // State FATAL
            }
        } else { // EXITED or restart
            if ((config.autorestart == task_config::TRUE) ||
                (config.autorestart == task_config::UNEXPECTED &&
                       !is_exited_normally(p))) {
                state.state = task_status::EXITED;
                task::start();
            } else {
                state.state = task_status::EXITED;
            }
        }
    }
}

// Returns true if the process completed successfully or was stopped by the user
bool task::is_exited_normally(proc::process p)
{
    if (!p.is_exited()) return false;
    return std::find(config.exitcodes.begin(), config.exitcodes.end(),
                     p.get_exitcode()) != config.exitcodes.cend();
}

/*
 * YAML Parser
 */

static unordered_map<string, void (*)(const YAML::Node &, task_config &)> _read_funcs_map = {
    {"prog",         _config_read_prog},
    {"args",         _config_read_args},
    {"numprocs",     _config_read_numprocs},
    {"umask",        _config_read_umask},
    {"workingdir",   _config_read_workingdir},
    {"autostart",    _config_read_autostart},
    {"autorestart",  _config_read_autorestart},
    {"exitcodes",    _config_read_exitcodes},
    {"startretries", _config_read_startretries},
    {"starttime",    _config_read_starttime},
    {"stopsignal",   _config_read_stopsignal},
    {"stoptime",     _config_read_stoptime},
    {"stdout",       _config_read_stdout},
    {"stderr",       _config_read_stderr},
    {"env",          _config_read_env},
};


static const unordered_map<string, int> _signal_names_map = {
    {"SIGHUP",    1 }, {"HUP",    1 },
    {"SIGINT",    2 }, {"INT",    2 },
    {"SIGQUIT",   3 }, {"QUIT",   3 },
    {"SIGILL",    4 }, {"ILL",    4 },
    {"SIGTRAP",   5 }, {"TRAP",   5 },
    {"SIGABRT",   6 }, {"ABRT",   6 },
    {"SIGFPE",    8 }, {"FPE",    8 },
    {"SIGKILL",   9 }, {"KILL",   9 },
    {"SIGBUS",    10}, {"BUS",    10},
    {"SIGSEGV",   11}, {"SEGV",   11},
    {"SIGSYS",    12}, {"SYS",    12},
    {"SIGPIPE",   13}, {"PIPE",   13},
    {"SIGALRM",   14}, {"ALRM",   14},
    {"SIGTERM",   15}, {"TERM",   15},
    {"SIGUSR1",   16}, {"USR1",   16},
    {"SIGUSR2",   17}, {"USR2",   17},
    {"SIGCHLD",   18}, {"CHLD",   18},
    {"SIGTSTP",   20}, {"TSTP",   20},
    {"SIGURG",    21}, {"URG",    21},
    {"SIGPOLL",   22}, {"POLL",   22},
    {"SIGSTOP",   23}, {"STOP",   23},
    {"SIGCONT",   25}, {"CONT",   25},
    {"SIGTTIN",   26}, {"TTIN",   26},
    {"SIGTTOU",   27}, {"TTOU",   27},
    {"SIGVTALRM", 28}, {"VTALRM", 28},
    {"SIGPROF",   29}, {"PROF",   29},
    {"SIGXCPU",   30}, {"XCPU",   30},
    {"SIGXFSZ",   31}, {"XFSZ",   31}
};

static const unordered_map<string, decltype(task_config::TRUE)> _autorestart_names_map = {
    {"false",      task_config::FALSE},
    {"unexpected", task_config::UNEXPECTED},
    {"true",       task_config::TRUE}
};

static void _config_read_prog(const YAML::Node &param, task_config &tconf)
{
    tconf.bin = param.as<string>();
}
static void _config_read_args(const YAML::Node &param, task_config &tconf)
{
    for (auto arg : param) tconf.args.push_back(arg.as<string>());
}
static void _config_read_numprocs(const YAML::Node &param, task_config &tconf)
{
    tconf.numprocs = param.as<size_t>();
}
static void _config_read_umask(const YAML::Node &param, task_config &tconf)
{
    tconf.mask = param.as<mode_t>();
}
static void _config_read_workingdir(const YAML::Node &param, task_config &tconf)
{
    tconf.workdir = param.as<string>();
}
static void _config_read_autostart(const YAML::Node &param, task_config &tconf)
{
    tconf.autostart = param.as<bool>();
}
static void _config_read_autorestart(const YAML::Node &param, task_config &tconf)
{
    auto it = _autorestart_names_map.find(param.as<string>());
    if (it != _autorestart_names_map.end())
        tconf.autorestart = it->second;
    else
        throw runtime_error("unexpected value: autorestart: " + param.as<string>());
}
static void _config_read_exitcodes(const YAML::Node &param, task_config &tconf)
{
    for (auto code : param) tconf.exitcodes.push_back(code.as<int>());
}
static void _config_read_startretries(const YAML::Node &param, task_config &tconf)
{
    tconf.startretries = param.as<size_t>();
}
static void _config_read_starttime(const YAML::Node &param, task_config &tconf)
{
    tconf.startsecs = param.as<time_t>();
}
static void _config_read_stopsignal(const YAML::Node &param, task_config &tconf)
{
    auto it = _signal_names_map.find(param.as<string>());
    if (it != _signal_names_map.end())
        tconf.stopsignal = it->second;
    else
        throw runtime_error("unexpected value: stopsignal: " + param.as<string>());
}
static void _config_read_stoptime(const YAML::Node &param, task_config &tconf)
{
    tconf.stopsecs = param.as<time_t>();
}
static void _config_read_stdout(const YAML::Node &param, task_config &tconf)
{
    tconf.stdout_file = param.as<string>();
}
static void _config_read_stderr(const YAML::Node &param, task_config &tconf)
{
    tconf.stderr_file = param.as<string>();
}
static void _config_read_env(const YAML::Node &param, task_config &tconf)
{
    for (auto env : param) tconf.envs.push_back(env.first.as<string>() + "=" +
                                                env.second.as<string>());
}

vector<task_config> tconfs_from_yaml(const std::string &file)
{
    try {
        YAML::Node config = YAML::LoadFile(file);

        if (config.IsNull()) throw YAML::Exception(YAML::Mark(), "file is empty");

        vector<task_config> task_cfgs;
        for (auto prog = config.begin(); prog != config.end(); ++prog) {
            task_config tconf;

            tconf.name = prog->first.as<std::string>();
            YAML::Node &params = prog->second;
            for (auto param = params.begin(); param != params.end(); ++param) {
                const string &param_name(param->first.as<string>());
                if (_read_funcs_map.find(param_name) != _read_funcs_map.end()) {
                    _read_funcs_map[param_name](param->second, tconf);
                } else {
                    clog << "Warning: Ignore unknown config parameter: " <<
                            tconf.name << ": " << param_name << endl;
                }
            }
            task_cfgs.push_back(tconf);
        }
        return task_cfgs;
    } catch (const YAML::BadFile &e) {
        clog << "Error while loading configuration file: " << file << ": " <<
                e.what() << endl <<
                "Configuration aborted, run reload config to retry." << endl;
    } catch (const YAML::Exception &e) {
        clog << "Error while parsing configuration file: " << file << ": " <<
                e.what() << endl <<
                "Configuration aborted, run reload config to retry." << endl;
    }
    return {};
}

void print_config(const task_config &tconf, ostream &stream)
{
    stream << "Name: " << tconf.name << endl;
    stream << "    Binary: " << tconf.bin << endl;
    stream << "    Args:" << endl;
    for (auto i : tconf.args) stream << "        " << i << endl;
    stream << "    Envs:" << endl;
    for (auto i : tconf.envs) stream << "        " << i << endl;
    stream << "    Numprocs: " << tconf.numprocs << endl;
    stream << "    Umask: 0" << oct << tconf.mask << dec << endl;
    stream << "    Work directory: " << tconf.workdir << endl;
    stream << "    Autostart: " << (tconf.autostart ? "true" : "false") << endl;
    switch (tconf.autorestart) {
    case task_config::FALSE:
        stream << "    Autorestart: false" << endl;
        break;
    case task_config::UNEXPECTED:
        stream << "    Autorestart: unexpected" << endl;
        break;
    case task_config::TRUE:
        stream << "    Autorestart: true" << endl;
        break;
    default:
        stream << "Error: Autorestart: Unexpected value!" << endl;
    }
    stream << "    Exitcodes:";
    for (auto i : tconf.exitcodes) stream << " " << i;
    stream << endl;
    stream << "    Startretries: " << tconf.startretries << endl;
    stream << "    Startseconds: " << tconf.startsecs << endl;
    stream << "    Stopsignal: " << tconf.stopsignal << endl;
    stream << "    Stopseconds: " << tconf.stopsecs << endl;
    stream << "    Stdin file: " << tconf.stdin_file << endl;
    stream << "    Stdout file: " << tconf.stdout_file << endl;
    stream << "    Stderr file: " << tconf.stderr_file << endl;
}

#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <exception>
#include <yaml-cpp/yaml.h>

#include "unistd.h"
#include "sys/types.h"

#include "task.hpp"

//using namespace tasks;

using namespace std;

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


task_config::task_config() : numprocs(DEFAULT_NUMPROC), mask(DEFAULT_MASK),
    workdir(DEFAULT_WORKDIR), autostart(DEFAULT_AUTOSTART),
    autorestart(DEFAULT_AUTORESTART), exitcodes(DEFAULT_EXIT_CODES),
    startretries(DEFAULT_STARTRETRIES), startsecs(DEFAULT_TIMETOSTART),
    stopsignal(DEFAULT_STOPSIG), stopsecs(DEFAULT_STOPTIME),
    stdin_file(DEFAULT_REDIR), stdout_file(DEFAULT_REDIR),
    stderr_file(DEFAULT_REDIR)
{}

task_status::task_status() : state(STOPPED), starttries(0), starttime(0)
{}

task::task(const task_config &configuration) : config(configuration)
{
    processes.resize(config.numprocs,
                     make_pair(proc::process(config.bin), task_status()));
    for (auto &proc: processes) {
        proc.first.set_args(config.args);
        proc.first.set_envs(config.envs);
        proc.first.set_workdir(config.workdir);
        proc.first.set_redirection(config.stdin_file, config.stdout_file,
                                   config.stderr_file);
        proc.first.set_stoptime(config.stopsecs);
        proc.first.set_umask(config.mask);
    }
    if (config.autostart) start();
}

void task::start()
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    for (size_t i = 0; i < config.numprocs; ++i)
        start(i);
}

void task::start(size_t index)
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
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
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    for (size_t i = 0; i < config.numprocs; ++i)
        stop(i);
}

void task::stop(size_t index)
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    if (index >= processes.size()) return;
    processes[index].first.stop(config.stopsignal);
    processes[index].second.state = task_status::STOPPED;
    processes[index].second.starttries = 0;
}

void task::restart()
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    stop();
    start();
}

void task::restart(size_t index)
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    if (index >= processes.size()) return;
    stop(index);
    start(index);
}

bool task::update()
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
    bool res = false;
    for (size_t i = 0; i < config.numprocs; ++i)
        res |= update(i);
    return res;
}

bool task::update(size_t index)
{
    clog << static_cast<const char *>(__PRETTY_FUNCTION__) << endl;
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
        } else if (config.autorestart == task_config::UNEXPECTED &&
                   !is_exited_normally(index)) {
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

#include <iostream>

#include <vector>
#include <string>
#include <unordered_map>
#include <exception>

#include <csignal>

#include "taskmaster.hpp"

using namespace std;

taskmaster *taskmaster::master_p = nullptr;

taskmaster::taskmaster(const std::string &file) : config_file(file)
{
    if (master_p) throw runtime_error("You cannot create more "
                                      "than one taskmaster object!");
    master_p = this;
    signal(SIGCHLD, update);
    load_yaml_config(config_file);
}

bool taskmaster::load_yaml_config(const string &file)
{
    config_file = file;

    clog << "Config file: " << file << endl;
    clear();
    auto tconfigs = tconfs_from_yaml(file);
    for (auto i : tconfigs) print_config(i, cout);
    for (auto t : tconfigs) emplace(t.name, t);
    return (configured = true);
}

string taskmaster::start(const std::string &name)
{
    auto t = find(name);
    if (t == end()) throw runtime_error("no such task");
    t->second.start();
    return name + ": started";
}

string taskmaster::stop(const std::string &name)
{
    auto t = find(name);
    if (t == end()) throw runtime_error("no such task");
    t->second.stop();
    return name + ": stopped";
}

string taskmaster::restart(const std::string &name)
{
    auto t = find(name);
    if (t == end()) throw runtime_error("no such task");
    t->second.restart();
    return name + ": restarted";
}

// An empty name returns the status of all programs
string taskmaster::status(const std::string &name)
{
    auto t = find(name);
    taskmaster::update(SIGCHLD);
    if (t != end()) return "status:\n" + t->second.status();
    if (empty()) return "no tasks\n";

    string s("status:\n");
    for (auto p : *this) {
        s += p.second.status();
    }
    return s;
}

string taskmaster::reload_config(const string &file)
{
    load_yaml_config(file.empty() ? config_file : file);
    return "config " + (file.empty() ? config_file : file) + " loaded";
}

string taskmaster::exit()
{
    std::exit(EXIT_SUCCESS);
}

void taskmaster::update(int /*signal*/)
{
    for (auto t : *master_p) t.second.update();
}

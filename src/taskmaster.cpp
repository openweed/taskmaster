#include <iostream>

#include <vector>
#include <string>
#include <unordered_map>
#include <exception>
#include <yaml-cpp/yaml.h>

#include "taskmaster.hpp"

using namespace std;

taskmaster::taskmaster()
{

}

bool taskmaster::load_yaml_config(const string &file)
{
    config_file = file;

    clog << "Config file: " << file << endl;

    auto tconfigs = tconfs_from_yaml(file);
    for (auto i : tconfigs) print_config(i, cout);
    for (auto t : tconfigs) emplace(t.name, t);
    return (configured = true);
}

void taskmaster::start(const std::string &name)
{
    cout << "taskmaster: start called for " << name << endl;
}

void taskmaster::stop(const std::string &name)
{
    auto t = find(name);
    if (t == end()) throw runtime_error("no such task");
    t->second.stop();
    cout << "taskmaster: stop called for " << name << endl;
}

void taskmaster::restart(const std::string &name)
{
    cout << "taskmaster: restart called for " << name << endl;
}

// An empty name returns the status of all programs
std::vector<task_status> taskmaster::status(const std::string &name)
{
    cout << "taskmaster: status called for " << (name.empty() ? "all" : name) <<
            endl;
    return {};
}

void taskmaster::reload_config(const string &file)
{
    cout << "taskmaster: reload config called for file: " <<
            (file.empty() ? "old file" : file) << endl;
}

void taskmaster::exit()
{
    cout << "taskmaster: exit called" << endl;
}

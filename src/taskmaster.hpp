#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include "defaults.hpp"
#include "task.hpp"

class taskmaster : public std::unordered_map<std::string, task>
{
public:
    taskmaster();
    bool load_yaml_config(const std::string &file);
    void start(const std::string &name);
    void stop(const std::string &name);
    void restart(const std::string &name);
    // An empty name returns the status of all programs
    std::vector<task_status> status(const std::string &name);
    // An empty name uses old config
    void reload_config(const std::string &file);
    void exit();
private:
    bool configured = false;
    std::string config_file;
};

#endif // CONFIGURATION_HPP

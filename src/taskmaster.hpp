#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <string>
#include <vector>
#include <unordered_map>

#include "master.hpp"
#include "task.hpp"

class taskmaster : public master, private std::unordered_map<std::string, task>
{
public:
    taskmaster() = default;
    bool load_yaml_config(const std::string &file);
    virtual void start(const std::string &name);
    virtual void stop(const std::string &name);
    virtual void restart(const std::string &name);
    // An empty name returns the status of all programs
    virtual std::vector<task_status> status(const std::string &name);
    // An empty name uses old config
    virtual void reload_config(const std::string &file);
    virtual void exit();
private:
    bool configured = false;
    std::string config_file;
};

#endif // CONFIGURATION_HPP

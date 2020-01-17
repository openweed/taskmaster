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
    ~taskmaster() {master_p = nullptr;}
    taskmaster(const std::string &file);
    bool load_yaml_config(const std::string &file);
    virtual std::string start(const std::string &name);
    virtual std::string stop(const std::string &name);
    virtual std::string restart(const std::string &name);
    // An empty name returns the status of all programs
    virtual std::string status(const std::string &name);
    // An empty name uses old config
    virtual std::string reload_config(const std::string &file);
    virtual std::string exit();
private:
    static void update(int signal = SIGCHLD);
    static taskmaster *master_p;
    bool configured = false;
    std::string config_file;
};

#endif // CONFIGURATION_HPP

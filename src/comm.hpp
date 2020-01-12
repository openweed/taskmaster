#ifndef COMM_HPP
#define COMM_HPP

#include <string>
#include <vector>

#include "task.hpp"

class comm
{
public:
    comm();
    void start(const std::string &name);
    void stop(const std::string &name);
    void restart(const std::string &name);
    // An empty name returns the status of all programs
    std::vector<task_status> status(const std::string &name);
    // An empty name uses old config
    void reload_config(const std::string &file);
    void exit();
private:
    bool send(const std::string cmd);
};

#endif // COMM_HPP

#ifndef MASTER_HPP
#define MASTER_HPP

#include <string>

#include"task.hpp"

class master
{
public:
    master() = default;
    virtual ~master() = default;
    virtual void start(const std::string &name) = 0;
    virtual void stop(const std::string &name) = 0;
    virtual void restart(const std::string &name) = 0;
    // An empty name returns the status of all programs
    virtual std::vector<task_status> status(const std::string &name) = 0;
    // An empty name uses old config
    virtual void reload_config(const std::string &file) = 0;
    virtual void exit() = 0;
};

#endif // MASTER_HPP

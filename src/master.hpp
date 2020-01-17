#ifndef MASTER_HPP
#define MASTER_HPP

#include <string>

#include"task.hpp"

class master
{
public:
    master() = default;
    virtual ~master() = default;
    virtual std::string start(const std::string &name) = 0;
    virtual std::string stop(const std::string &name) = 0;
    virtual std::string restart(const std::string &name) = 0;
    // An empty name returns the status of all programs
    virtual std::string status(const std::string &name) = 0;
    // An empty name uses old config
    virtual std::string reload_config(const std::string &file) = 0;
    virtual std::string exit() = 0;
};

#endif // MASTER_HPP

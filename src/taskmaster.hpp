#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <string>
#include <vector>

#include "defaults.hpp"
#include "task.hpp"

class taskmaster
{
public:
    taskmaster();
    bool load_config(const std::vector<task_config> config);
private:
    bool configured = false;
    std::vector<task> tasks;
};

#endif // CONFIGURATION_HPP

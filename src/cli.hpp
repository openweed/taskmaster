#ifndef CLI_HPP
#define CLI_HPP

#include <iostream>
#include <sstream>
#include <unordered_map>

#include "master.hpp"

static constexpr auto CLI_PROMPT = "taskmaster> ";
static constexpr auto CLI_USAGE = "Available commands:\n"
                                  "    start NAME\n"
                                  "    stop NAME\n"
                                  "    restart NAME\n"
                                  "    status [NAME]\n"
                                  "    reload-config [FILE]\n"
                                  "    exit [cli|daemon]\n";

namespace  {
enum cmd_types {
    CMD_START,
    CMD_STOP,
    CMD_RESTART,
    CMD_STATUS,
    CMD_RELOAD_CONFIG,
    CMD_EXIT
};
}

class cli
{
public:
    cli(master &worker) : worker(worker) {}
    int run();
private:
    master &worker;
    void cmd_start(std::istringstream &args);
    void cmd_stop(std::istringstream &args);
    void cmd_restart(std::istringstream &args);
    void cmd_status(std::istringstream &args);
    void cmd_reload_config(std::istringstream &args);
    void cmd_exit(std::istringstream &args);
};

#endif // CLI_HPP

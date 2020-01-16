#ifndef CLI_HPP
#define CLI_HPP

#include <iostream>
#include <sstream>
#include <unordered_map>

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

template<class T>
class cli
{
public:
    cli(T &processor) : master(processor) {}
    int run();
private:
    T &master;
    static std::unordered_map<std::string, cmd_types> cmd_map;
    void cmd_start(std::istringstream &args);
    void cmd_stop(std::istringstream &args);
    void cmd_restart(std::istringstream &args);
    void cmd_status(std::istringstream &args);
    void cmd_reload_config(std::istringstream &args);
    void cmd_exit(std::istringstream &args);
};

template<class T>
std::unordered_map<std::string, cmd_types> cli<T>::cmd_map = {
        {"start",         CMD_START},
        {"stop",          CMD_STOP},
        {"restart",       CMD_RESTART},
        {"status",        CMD_STATUS},
        {"reload-config", CMD_RELOAD_CONFIG},
        {"exit",          CMD_EXIT}
};

template<class T>
int cli<T>::run()
{
    for (std::string line; (std::cout << CLI_PROMPT, std::getline(std::cin, line));) {
        std::istringstream cmd_stream(line);
        std::string cmd;
        if (!(cmd_stream >> cmd)) continue;

        auto cmd_type = cmd_map.find(cmd);
        if (cmd_type == cmd_map.end()) {
            std::cerr << "Unknown command: " << cmd << std::endl << CLI_USAGE
                   << std::flush;
            continue;
        }

        switch (cmd_type->second) {
        case CMD_START:
            cmd_start(cmd_stream);
            break;
        case CMD_STOP:
            cmd_stop(cmd_stream);
            break;
        case CMD_RESTART:
            cmd_restart(cmd_stream);
            break;
        case CMD_STATUS:
            cmd_status(cmd_stream);
            break;
        case CMD_RELOAD_CONFIG:
            cmd_reload_config(cmd_stream);
            break;
        case CMD_EXIT:
            cmd_exit(cmd_stream);
            break;
        default:
            std::cerr << "Unknown error while parsing command." << std::endl;
        }
    }
    return 0;
}

template<class T>
void cli<T>::cmd_start(std::istringstream &args)
{
    std::string name;
    if (!(args >> name)) {
        std::cerr << "Usage: start NAME" << std::endl;
        return;
    }
    master.start(name);
}

template<class T>
void cli<T>::cmd_stop(std::istringstream &args)
{
    std::string name;
    if (!(args >> name)) {
        std::cerr << "Usage: stop NAME" << std::endl;
        return;
    }
    master.stop(name);
}

template<class T>
void cli<T>::cmd_restart(std::istringstream &args)
{
    std::string name;
    if (!(args >> name)) {
        std::cerr << "Usage: restart NAME" << std::endl;
        return;
    }
    master.restart(name);
}

template<class T>
void cli<T>::cmd_status(std::istringstream &args)
{
    std::string name;
    args >> name; // empty -> all
    master.status(name);
}

template<class T>
void cli<T>::cmd_reload_config(std::istringstream &args)
{
    std::string file;
    args >> file; // empty -> old config
    master.reload_config(file);
}

template<class T>
void cli<T>::cmd_exit(std::istringstream &args)
{
    std::string name;
    if (!(args >> name)) std::exit(0);

    if (name == "daemon") {
        master.exit();
    } else if (name == "cli") {
        std::exit(0);
    } else {
        std::cerr << std::endl << "Usage: exit [cli|daemon]" << std::endl;
    }
}

#endif // CLI_HPP

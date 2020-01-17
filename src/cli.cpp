#include "cli.hpp"

static std::unordered_map<std::string, cmd_types> _cmd_map = {
        {"start",         CMD_START},
        {"stop",          CMD_STOP},
        {"restart",       CMD_RESTART},
        {"status",        CMD_STATUS},
        {"reload-config", CMD_RELOAD_CONFIG},
        {"exit",          CMD_EXIT}
};

int cli::run()
{
    for (std::string line; (std::cout << CLI_PROMPT, std::getline(std::cin, line));) {
        std::istringstream cmd_stream(line);
        std::string cmd;
        if (!(cmd_stream >> cmd)) continue;

        auto cmd_type = _cmd_map.find(cmd);
        if (cmd_type == _cmd_map.end()) {
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

void cli::cmd_start(std::istringstream &args)
{
    std::string name;
    if (!(args >> name)) {
        std::cerr << "Usage: start NAME" << std::endl;
        return;
    }
    worker.start(name);
}

void cli::cmd_stop(std::istringstream &args)
{
    std::string name;
    if (!(args >> name)) {
        std::cerr << "Usage: stop NAME" << std::endl;
        return;
    }
    worker.stop(name);
}

void cli::cmd_restart(std::istringstream &args)
{
    std::string name;
    if (!(args >> name)) {
        std::cerr << "Usage: restart NAME" << std::endl;
        return;
    }
    worker.restart(name);
}

void cli::cmd_status(std::istringstream &args)
{
    std::string name;
    args >> name; // empty -> all
    worker.status(name);
}

void cli::cmd_reload_config(std::istringstream &args)
{
    std::string file;
    args >> file; // empty -> old config
    worker.reload_config(file);
}

void cli::cmd_exit(std::istringstream &args)
{
    std::string name;
    if (!(args >> name)) std::exit(0);

    if (name == "daemon") {
        worker.exit();
    } else if (name == "cli") {
        std::exit(0);
    } else {
        std::cerr << std::endl << "Usage: exit [cli|daemon]" << std::endl;
    }
}

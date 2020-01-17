#include "cli.hpp"

using namespace std;

static unordered_map<string, cmd_types> _cmd_map = {
        {"start",         CMD_START},
        {"stop",          CMD_STOP},
        {"restart",       CMD_RESTART},
        {"status",        CMD_STATUS},
        {"reload-config", CMD_RELOAD_CONFIG},
        {"exit",          CMD_EXIT}
};

int cli::run()
{
    for (string line; (cout << CLI_PROMPT, getline(cin, line));) {
        istringstream cmd_stream(line);
        string cmd;
        if (!(cmd_stream >> cmd)) continue;

        auto cmd_type = _cmd_map.find(cmd);
        if (cmd_type == _cmd_map.end()) {
            cerr << "Unknown command: " << cmd << endl << CLI_USAGE
                   << flush;
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
            cerr << "Unknown error while parsing command." << endl;
        }
    }
    return 0;
}

void cli::cmd_start(istringstream &args)
{
    string name;
    if (!(args >> name)) {
        cerr << "Usage: start NAME" << endl;
        return;
    }
    try {
        auto res = worker.start(name);
        if(!res.empty()) cout << res << endl;
    } catch (const exception &e) {
        cerr << name << ": error: " << e.what() << endl;
    }
}

void cli::cmd_stop(istringstream &args)
{
    string name;
    if (!(args >> name)) {
        cerr << "Usage: stop NAME" << endl;
        return;
    }
    try {
        auto res = worker.stop(name);
        if(!res.empty()) cout << res << endl;
    } catch (const exception &e) {
        cerr << name << ": error: " << e.what() << endl;
    }
}

void cli::cmd_restart(istringstream &args)
{
    string name;
    if (!(args >> name)) {
        cerr << "Usage: restart NAME" << endl;
        return;
    }
    try {
        auto res = worker.restart(name);
        if(!res.empty()) cout << res << endl;
    } catch (const exception &e) {
        cerr << name << ": error: " << e.what() << endl;
    }
}

void cli::cmd_status(istringstream &args)
{
    string name;
    args >> name; // empty -> all
    try {
        auto res = worker.status(name);
        if(!res.empty()) cout << res;
    } catch (const exception &e) {
        cerr << name << ": error: " << e.what() << endl;
    }
}

void cli::cmd_reload_config(istringstream &args)
{
    string file;
    args >> file; // empty -> old config
    auto res = worker.reload_config(file);
    try {
        if(!res.empty()) cout << res << endl;
    } catch (const exception &e) {
        cerr << "error: " << e.what() << endl;
    }
}

void cli::cmd_exit(istringstream &args)
{
    string name;
    if (!(args >> name)) exit(0);

    if (name == "daemon") {
        try {
            auto res = worker.exit();
            if(!res.empty()) cout << res << endl;
        } catch (const exception &e) {
            cerr << "error: " << e.what() << endl;
        }
    } else if (name == "cli") {
        exit(0);
    } else {
        cerr << endl << "Usage: exit [cli|daemon]" << endl;
    }
}

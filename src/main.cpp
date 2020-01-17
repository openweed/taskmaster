#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <rapidjson/rapidjson.h>

//#include "process.hpp"
#include "task.hpp"
#include "taskmaster.hpp"
#include "cli.hpp"
#include "communication.hpp"

//Common defines
const char *const shortopts = "+hdcp:a:";
static const std::array<option, 7> longopts {
    option({"help", no_argument, nullptr, 'h'}),
    option({"daemon", no_argument, nullptr, 'd'}),
    option({"cli", no_argument, nullptr, 'c'}),
    option({"logfile", required_argument, nullptr, 1}),
    option({"port", required_argument, nullptr, 'p'}),
    option({"address", required_argument, nullptr, 'a'}),
    option({nullptr, 0, nullptr, 0})
};
///

using namespace std;
using namespace proc;

string logfile;
bool daemon_mode = 0;
bool client_mode = 0;
unsigned int port = 4242;
string address = "localhost";

void usage()
{
    cerr << "usage: taskmaster [-h] [--logfile log_file] [-d | --daemon]\n"
            "                  [-c | --cli] [-p port | --port=port]\n"
            "                  [-a address | --address=address]" << endl;
}

int parse_opt(int argc, char **argv)
{
    while (true) {
        switch (getopt_long(argc, argv, shortopts, longopts.data(), nullptr)) {
        case -1:
            return 0;
        case 1:                   // --logfile
            logfile = optarg;
            break;
        case 'd':                 // -d, --daemon
            client_mode = 0;
            daemon_mode = 1;
            break;
        case 'c':                 // -c, --cli
            daemon_mode = 0;;
            client_mode = 1;
            break;
        case 'p':                 // -c, --cli
            port = stoi(optarg);
            break;
        case 'a':                 // -c, --cli
            address = optarg;
            break;
        case '?':                 // -?, unknown option
        case 'h':                 // -h, --help
        default:
            usage();
            return 1;
        }
    }
    return 0;
}

int main(int  argc, char *argv[])
{
    try {
        if (parse_opt(argc, argv)) return 1;
        if (daemon_mode) {
            taskmaster master("/home/user/Projects/taskmaster/config.yaml");
            cout << port << endl;
            communication comm(&master, port);
            comm.run_master();
        } else if (client_mode) {
            communication comm(nullptr, port, address);
            cli console(comm);
            console.run();
        } else {
            taskmaster master("/home/user/Projects/taskmaster/config.yaml");
            cli console(master);
            console.run();
        }
    } catch (const exception &e) {
        cerr << "fatal error: " << e.what() << endl;
    }
    return 0;
}

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <rapidjson/rapidjson.h>

#include <unistd.h>

#include "taskmaster.hpp"
#include "cli.hpp"
#include "communication.hpp"

//Common defines
const char *const shortopts = "+hdcp:a:";
static const std::array<option, 8> longopts {
    option({"help", no_argument, nullptr, 'h'}),
    option({"daemon", no_argument, nullptr, 'd'}),
    option({"cli", no_argument, nullptr, 'c'}),
    option({"logfile", required_argument, nullptr, 1}),
    option({"config", required_argument, nullptr, 2}),
    option({"port", required_argument, nullptr, 'p'}),
    option({"address", required_argument, nullptr, 'a'}),
    option({nullptr, 0, nullptr, 0})
};
///

using namespace std;
using namespace proc;

string logfile;
string conffile;
bool daemon_mode = 0;
bool client_mode = 0;
unsigned int port = 4242;
string address = "localhost";

void usage()
{
    cerr << "usage: taskmaster [-h] [--logfile log_file] [-d | --daemon]\n"
            "                  [-c | --cli] [-p port | --port=port]\n"
            "                  [-a address | --address=address]\n"
            "                  [--config=config_file]" << endl;
}

void check_daemon()
{
    ifstream pidfile("/tmp/taskmaster.pid", ios::in);
    pid_t pid = 0;
    pidfile >> pid;
    if (pid && !kill(pid, 0))
        throw runtime_error("the daemon is already running\n"
                            "If not, delete /tmp/taskmaster.pid");
}

void daemonize()
{
    check_daemon();
    daemon(true, true);
    ofstream pidfile("/tmp/taskmaster.pid");
    pidfile << getpid();
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
        case 2:                   // --logfile
            conffile = optarg;
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
    if (parse_opt(argc, argv)) return 1;
    if (conffile.empty()) conffile = "/tmp/taskmaster.yaml";
    ofstream log(logfile, ios::app);
    clog.rdbuf(log.rdbuf());
    try {
        if (daemon_mode) {
            daemonize();
            taskmaster master(conffile);
            communication comm(&master, port);
            comm.run_master();
        } else if (client_mode) {
            communication comm(nullptr, port, address);
            cli console(comm);
            console.run();
        } else {
            check_daemon();
            taskmaster master(conffile);
            cli console(master);
            console.run();
        }
    } catch (const exception &e) {
        clog.rdbuf(nullptr);
        cerr << "fatal error: " << e.what() << endl;
    }
    return 0;
}

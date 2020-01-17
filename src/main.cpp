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
const char *const shortopts = "+hdc";
static const std::array<option, 5> longopts {
    option({"help", no_argument, nullptr, 'h'}),
    option({"daemon", no_argument, nullptr, 'd'}),
    option({"cli", required_argument, nullptr, 'c'}),
    option({"logfile", required_argument, nullptr, 1}),
    option({nullptr, 0, nullptr, 0})
};
///

using namespace std;
using namespace proc;

string logfile;
int run_daemon = 0;
int run_client = 0;
int run_command = 1;

void usage(const std::string &progname)
{
    cerr << "usage: " << progname << " [-h] [--logfile log_file] -d | --daemon" << endl;
    cerr << "       " << progname << " [-h] [--logfile log_file] -c | --cli" << endl;
    cerr << "       " << progname << " [-h] [--logfile log_file] -- cmd" << endl;
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
            run_command = run_client = 0;
            run_daemon = 1;
            break;
        case 'c':                 // -c, --cli
            run_command = run_daemon = 0;;
            run_client = 1;
            break;
        case '?':                 // -?, unknown option
        default:
        case 'h':                 // -h, --help
            usage(argc ? *argv : "program");
            return 1;
        }
    }
    return 0;
}

int main(int  argc, char *argv[])
{
    parse_opt(argc, argv);
    cout << "Cli:" << run_client << endl;
    cout << "Daemon:" << run_daemon << endl;
    if (run_daemon) {
        taskmaster master("/home/user/Projects/taskmaster/config.yaml");

        communication comm(&master);
        comm.run_master();
    } else if (run_client) {
        communication comm(nullptr);

        cli cmd(comm);
        cmd.run();
    } else {
        taskmaster master;
        master.load_yaml_config("/home/user/Projects/taskmaster/config.yaml");

        cli cmd(master);
        cmd.run();
    }

    return 0;
}

////    cout << "Hello World!" << endl;

////    if (parse_opt(argc, argv)) exit(1);
////    cout << "daemon:" << as_daemon << endl;
////    cout << "cli:" << as_cli << endl;
////    cout << "log:" << logfile << endl;

////    process proc("/bin/ls");
////    proc.set_args({"-l", "-R"});
////    proc.set_workdir("/var/log");
//////    proc.set_workdir("/");
////    proc.set_redirection("", "/tmp/ls.out", "/tmp/ls.err");
////    proc.start();
////    sleep(2);
////    proc.stop();
////    cout << "Next" << endl;
////    proc.start();
////    while (proc.is_exist()) proc.update();

//    tasks::task_config config;
//    config.name = "ls";
//    config.bin = "/bin/ls";
//    config.args = {"-l", "-R"};
////    config.directory = "/home/user";
//    config.directory = "/var/log";
//    config.stdout_file = "/tmp/ls.out";
//    config.stderr_file = "/tmp/ls.err";
////    config.numproc = 10;

//    tasks::task ps(config);
//    ps.start();
//    sleep(2);
//    ps.start();

////    sleep(11);
//    wait(nullptr);
//    wait(nullptr);
//    wait(nullptr);

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <rapidjson/rapidjson.h>

//#include "process.hpp"
#include "task.hpp"

using namespace std;

using namespace proc;

int main()
{
    cout << "Hello World!" << endl;

//    process proc("/bin/ls");
//    proc.set_args({"-l", "-R"});
//    proc.set_workdir("/var/log");
//    proc.set_redirection("", "/tmp/ls.out", "/tmp/ls.err");
//    proc.start();
//    sleep(1);
//    proc.start();
//    proc.start();
//    proc.start();
    tasks::task_config config;
    config.name = "ls";
    config.bin = "/bin/ls";
    config.args = {"-l", "-R"};
//    config.workdir = "/home/user";
    config.workdir = "/var/log";
    config.stdout_file = "/tmp/ls.out";
    config.stderr_file = "/tmp/ls.err";
//    config.numproc = 20;

    tasks::task ps(config);
    ps.start();

    waitpid(-1, nullptr, 0);
    return 0;
}

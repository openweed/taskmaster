#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <rapidjson/rapidjson.h>

#include "process.hpp"

using namespace std;

int main()
{
    cout << "Hello World!" << endl;

    process proc("/bin/ls");
    proc.set_args({"-l", "-R"});
    proc.set_workdir("/var/log");
    proc.set_redirection("", "/tmp/ls.out", "/tmp/ls.err");
    proc.start();
    waitpid(-1, 0, 0);
    return 0;
}

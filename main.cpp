#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <process.hpp>

using namespace std;

int main()
{
    cout << "Hello World!" << endl;

    process proc("/bin/ls", {"-l", "-R"});
    proc.set_dir("/var/log");
    proc.start();
    waitpid(-1, 0, 0);
    return 0;
}

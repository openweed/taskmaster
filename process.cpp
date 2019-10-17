#include "process.hpp"

#include <cstring>

using namespace std;

process::process(const std::string program_path, const std::vector<std::string> args, bool run = false) :
    path(program_path), argv(nullptr), envp(nullptr)
{
    set_args(args);
}
process::~process()
{
    free_args();
}

void process::set_args(const std::vector<const std::string> args)
{
    if (!args.size()) {
        free_args();
        return;
    }
    argv = new char *[args.size() + 1];
    size_t i = 0;
    for (auto iter = args.cbegin(); iter != args.cend(); ++iter)
        argv[i++] = strdup(iter->c_str());
    argv[i] = nullptr;
}

/*
 * Private
 */

void process::free_args()
{
    if (!argv) return;
    char **tmp = argv;
    while (*tmp) free(*tmp++);
    free(argv);
}

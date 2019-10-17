#include "process.hpp"

#include <cstring>

using namespace std;

process::process(const string &program_path,
                 const std::vector<std::string> &args,
                 bool run) :
    path(program_path), dir("/")
{
    argv = nullptr;
    envp = nullptr;
    state = PROCESS_STATE_STOPEED;
    exit_code = 0;

    set_args(args);
}

process::~process()
{
    free_argv();
}

/*
 * Запускает процесс и добавляет его в группу group, если group 0, то устанавливается группа pid
 * Возвращает pid нового процесса
 */
pid_t process::start()
{
    if (pid = fork()) return pid;
    if (pid == -1) {
        if (errno == EAGAIN) return start();
        return -errno;
    }
    // XXX apply redirection
    // XXX maybe change pgroup
    chdir(dir.c_str());
    execve(path.c_str(), argv, envp);
    exit(1);
}

void process::set_args(const std::vector<string> &args)
{
    free_argv();
    argv = static_cast<char **>(malloc(sizeof(char *) * (args.size() + 2)));
    if (!argv) throw bad_alloc();

    size_t i = 0;
    if (!(argv[i++] = strdup(path.c_str()))) {
        free_argv();
        throw bad_alloc();
    }
    for (auto iter = args.cbegin(); iter != args.cend(); ++iter) {
        if (!(argv[i++] = strdup(iter->c_str()))) {
            free_argv();
            throw bad_alloc();
        }
    }
    argv[i] = nullptr;
}

/*
 * Private
 */

void process::free_argv()
{
    if (!argv) return;
    char **tmp = argv;
    while (*tmp) free(*tmp++);
    free(argv);
}

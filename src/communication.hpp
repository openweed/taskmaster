#ifndef COMM_HPP
#define COMM_HPP

#include <string>
#include <vector>

#include <zmq.hpp>

#include "taskmaster.hpp"

constexpr unsigned int TDAEMON_PORT = 4242;

class communication
{
private:
    enum class msg_type
    {
        REQ_START,
        REQ_STOP,
        REQ_RESTART,
        REQ_RELOAD_CONFIG,
        REQ_EXIT,
        REPLY
    };
    struct msg_hdr {
        msg_type type;
        std::size_t total_len;
        char data[0];
    } __attribute__((packed));
public:
    communication(bool master);
    void run_master(taskmaster &master);
    void start(const std::string &name);
    void stop(const std::string &name);
    void restart(const std::string &name);
    // An empty name returns the status of all programs
    std::vector<task_status> status(const std::string &name);
    // An empty name uses old config
    void reload_config(const std::string &file);
    void exit();
private:
    zmq::context_t context;
    zmq::socket_t socket;
    void connect();
    void bind();
    void send(msg_hdr *msg);
};

#endif // COMM_HPP

#ifndef COMM_HPP
#define COMM_HPP

#include <string>
#include <vector>
#include <memory>

#include <zmq.hpp>

#include "taskmaster.hpp"

constexpr unsigned int TDAEMON_PORT = 4242;

class communication
{
private:
    enum class msg_type : int
    {
        REQ_START,
        REQ_STOP,
        REQ_RESTART,
        REQ_STATUS,
        REQ_RELOAD_CONFIG,
        REQ_EXIT,
        REP_REP,
        REP_ERR,
    };
    struct msg_hdr {
        msg_type type;
        std::size_t total_len;
        char data[0];
    } __attribute__((packed));
public:
    communication(bool is_master, taskmaster *master_p = nullptr);
    void run_master();

    void start(const std::string &name);
    void stop(const std::string &name);
    void restart(const std::string &name);
    // An empty name returns the status of all programs
    std::vector<task_status> status(const std::string &name);
    // An empty name uses old config
    void reload_config(const std::string &file);
    void exit();
private:
    std::unique_ptr<msg_hdr, void(*)(msg_hdr *)>
    get_raw_msg(std::size_t content_size);
    void send(msg_hdr *msg);
    void send_str(const std::string &str, msg_type type);

    void rep_start(const std::string &name);
    void rep_stop(const std::string &name);
    void rep_restart(const std::string &name);
    void rep_status(const std::string &name);
    void rep_reload_config(const std::string &name);
    void rep_exit();

    void cli_get_reply();
    taskmaster *master;
    zmq::context_t context;
    zmq::socket_t socket;

};

#endif // COMM_HPP

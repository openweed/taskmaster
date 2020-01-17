#ifndef COMM_HPP
#define COMM_HPP

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

#include <zmq.hpp>

#include "master.hpp"
#include "taskmaster.hpp"

constexpr unsigned int TDAEMON_PORT = 4242;
constexpr int          TCLI_SNDTIMEO = 0;
constexpr int          TCLI_RCVTIMEO = 1000;

class communication : public master, private zmq::context_t, zmq::socket_t, zmq::monitor_t
{
private:
    enum class msg_type : int {
        MIN_REQ = 0,
        REQ_START = MIN_REQ,
        REQ_STOP,
        REQ_RESTART,
        REQ_STATUS,
        REQ_RELOAD_CONFIG,
        REQ_EXIT,
        MAX_REQ = REQ_EXIT,
        MIN_REP,
        REP_REP = MIN_REP,
        REP_ERR,
        MAX_REP = REP_ERR
    };
    struct msg_hdr {
        msg_type type;
        std::size_t total_len;
        char data[0];
    } __attribute__((packed));
public:
    communication(taskmaster *master_p,
                  unsigned int port = TDAEMON_PORT,
                  const std::string address = "localhost");
    ~communication();
    void run_master();

    virtual std::string start(const std::string &name);
    virtual std::string stop(const std::string &name);
    virtual std::string restart(const std::string &name);
    // An empty name returns the status of all programs
    virtual std::string status(const std::string &name);
    // An empty name uses old config
    virtual std::string reload_config(const std::string &file);
    virtual std::string exit();
private:
    std::unique_ptr<msg_hdr, void(*)(msg_hdr *)>
    get_raw_msg(std::size_t content_size);
    size_t send_msg(msg_hdr *msg);
    size_t send_str(const std::string &str, msg_type type);

    // Cli members
    std::atomic_bool connected = false;
    std::thread monitor_thread;
    std::mutex  monitor_mutex;
    void monitor_init();
    virtual void on_monitor_started();
    virtual void on_event_connected(const zmq_event_t &event_, const char* addr_);
    virtual void on_event_disconnected(const zmq_event_t &event_, const char* addr_);
    size_t send_req(const std::string &name, msg_type req);
    std::string get_reply();

    // Master members
    taskmaster *master = nullptr;
    size_t send_rep(const std::string &str, msg_type rep);
    void rep_start(const std::string &name);
    void rep_stop(const std::string &name);
    void rep_restart(const std::string &name);
    void rep_status(const std::string &name);
    void rep_reload_config(const std::string &file);
    void rep_exit();
};

#endif // COMM_HPP

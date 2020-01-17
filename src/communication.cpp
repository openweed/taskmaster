#include <iostream>
#include <memory>
#include <cstring>
#include <cstdlib>

#include "communication.hpp"

using namespace std;

communication::communication(taskmaster *master_p, unsigned int port,
                             const std::string address) :
    context_t(1),
    socket_t(*this, (master_p ? ZMQ_REP : ZMQ_REQ)),
    master(master_p)
{
    if (master_p) {
        try {
            bind("tcp://*:" + to_string(port));
            connected = true;
        } catch (const exception &e) {
            clog << "Connection initialization error: " << e.what() << endl <<
                    "The program will run in uncontrolled mode." << endl;
        }
    } else {
        try {
            monitor_init();
            connect("tcp://" + address + ":" + to_string(port));
        } catch (const exception &e) {
            cerr << "Connection failed: " << e.what() << endl;
            std::exit(EXIT_FAILURE);
        }
    }
}

communication::~communication()
{
    zmq::monitor_t::abort();
    monitor_thread.join();
}

void communication::run_master()
{
    clog << "The daemon is running on port " << to_string(TDAEMON_PORT) << endl;
    while (true) {
        zmq::message_t request;
        recv(&request);
        msg_hdr *msg = static_cast<msg_hdr *>(request.data());
        clog << "Recived message: type " << static_cast<int>(msg->type) << endl;
        switch (msg->type) {
        case msg_type::REQ_START:
            rep_start(msg->data);
            break;
        case msg_type::REQ_STOP:
            rep_stop(msg->data);
            break;
        case msg_type::REQ_RESTART:
            rep_restart(msg->data);
            break;
        case msg_type::REQ_STATUS:
            rep_status(msg->data);
            break;
        case msg_type::REQ_RELOAD_CONFIG:
            rep_reload_config(msg->data);
            break;
        case msg_type::REQ_EXIT:
            rep_exit();
            break;
        default:;
            send_str("error: invalid message type", msg_type::REP_ERR);
        }
    }
}

string communication::start(const std::string &name)
{
    if (send_req(name, msg_type::REQ_START))
        return get_reply();
    return "";
}

string communication::stop(const std::string &name)
{
    if (send_req(name, msg_type::REQ_STOP))
        return get_reply();
    return "";
}

string communication::restart(const std::string &name)
{
    if (send_req(name, msg_type::REQ_RESTART))
        return get_reply();
    return "";
}

string communication::status(const std::string &name)
{
    if (send_req(name, msg_type::REQ_STATUS))
        return get_reply();
    return "";
}

string communication::reload_config(const std::string &file)
{
    if (send_req(file, msg_type::REQ_RELOAD_CONFIG))
        return get_reply();
    return "";
}

string communication::exit()
{
    if (send_req("", msg_type::REQ_EXIT))
        return get_reply();
    return "";
}

string communication::get_reply()
{
    zmq::message_t reply;
    recv(&reply);
    if (reply.size() < sizeof (msg_hdr)) {
        return "error: recived incorrect message";
    }

    msg_hdr *msg = static_cast<msg_hdr *>(reply.data());
    if (msg->type == msg_type::REP_REP || msg->type == msg_type::REP_ERR)
        return string("daemon: ") + msg->data;
    else
        return "error: recived incorrect message";
}

std::unique_ptr<communication::msg_hdr, void(*)(communication::msg_hdr*)>
communication::get_raw_msg(size_t content_size)
{
    size_t msg_len = sizeof(msg_hdr) + content_size;
    auto deleter = [](msg_hdr *p) {delete [] reinterpret_cast<char *>(p);};
    unique_ptr<msg_hdr, decltype (deleter)>
            raw_msg(reinterpret_cast<msg_hdr *>(new char[msg_len]), deleter);
    msg_hdr *msg = reinterpret_cast<msg_hdr *>(raw_msg.get());
    msg->total_len = msg_len;
    return raw_msg;
}

size_t communication::send_msg(msg_hdr *msg)
{
    if (connected) {
        return send(msg, msg->total_len, ZMQ_DONTWAIT);
    } else {
        cout << "No connection to the daemon." << endl <<
                "Run 'taskmaster --daemon' in the terminal "
                "to start the daemon." << endl;
        return 0;
    }
}

size_t communication::send_str(const string &str, msg_type type)
{
    auto raw_msg = get_raw_msg(str.size() + 1);
    msg_hdr *msg = raw_msg.get();

    msg->type = type;
    strcpy(msg->data, str.c_str());
    return send_msg(msg);
}

size_t communication::send_req(const string &name, msg_type req)
{
    if (req < msg_type::MIN_REQ || req > msg_type::MAX_REQ)
        throw runtime_error("fatal error");
    return send_str(name, req);
}

size_t communication::send_rep(const string &str, msg_type rep)
{
    if (rep < msg_type::MIN_REP || rep > msg_type::MAX_REP)
        throw runtime_error("fatal error");
    return send_str(str, rep);
}

void communication::monitor_init()
{
    auto monitor_func = [this]()
    {
        this->monitor(*this, "inproc://monitor-client",
                      ZMQ_EVENT_CONNECTED | ZMQ_EVENT_DISCONNECTED);
    };
    monitor_mutex.lock();
    monitor_thread = thread(monitor_func);
    monitor_mutex.lock();
    monitor_mutex.unlock();
}

void communication::on_monitor_started()
{
    monitor_mutex.unlock();
}

void communication::on_event_connected(const zmq_event_t &, const char*)
{
    connected = true;
}

void communication::on_event_disconnected(const zmq_event_t &, const char*)
{
    connected = false;
}

void communication::rep_start(const std::string &name)
{
    try {
        send_rep(master->start(name), msg_type::REP_REP);
    } catch (const exception &e) {
        send_rep(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_stop(const std::string &name)
{
    try {
        send_rep(master->stop(name), msg_type::REP_REP);
    } catch (const exception &e) {
        send_rep(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_restart(const std::string &name)
{
    try {
        send_rep(master->restart(name), msg_type::REP_REP);
    } catch (const exception &e) {
        send_rep(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_status(const std::string &name)
{
    try {
        send_rep(master->status(name), msg_type::REP_REP);
    } catch (const exception &e) {
        send_rep(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_reload_config(const std::string &file)
{
    try {
        send_rep(master->reload_config(file), msg_type::REP_REP);
    } catch (const exception &e) {
        send_rep(file + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_exit()
{
    send_rep("goodbye", msg_type::REP_REP);
    master->exit();
}

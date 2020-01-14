#include <iostream>
#include <memory>
#include <cstring>
#include <cstdlib>

#include "communication.hpp"

using namespace std;

communication::communication(bool is_master, taskmaster *master_p) :
    master(master_p),
    context(1),
    socket(context, (is_master ? ZMQ_REP : ZMQ_REQ))
{
    try {
        if (is_master) {
            if (!master_p) throw runtime_error("fatal error");
            socket.bind((string("tcp://*:") + to_string(TDAEMON_PORT)));
        } else {
            socket.connect((string("tcp://localhost:") + to_string(TDAEMON_PORT)));
        }
    } catch (const exception &e) {
        if (is_master) {
            clog << "Connection initialization error: " << e.what() << endl <<
                    "The program will run in uncontrolled mode." << endl;
        } else {
            cerr << "Connection failed: " << e.what() << endl;
            std::exit(EXIT_FAILURE);
        }
    }
}

void communication::run_master()
{
    clog << "The daemon is running on port " << to_string(TDAEMON_PORT) << endl;
    while (true) {
        zmq::message_t request;
        socket.recv(&request);
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

void communication::start(const std::string &name)
{
    auto raw_msg = get_raw_msg(name.size() + 1);
    msg_hdr *msg = raw_msg.get();

    msg->type = msg_type::REQ_START;
    strcpy(msg->data, name.c_str());
    send(msg);

    cli_get_reply();
}
void communication::stop(const std::string &name)
{
    cout << "communication: stop called for " << name << endl;
}
void communication::restart(const std::string &name)
{
    cout << "communication: restart called for" << name << endl;
}
std::vector<task_status> communication::status(const std::string &name)
{
    cout << "communication: status called for " << (name.empty() ? "all" : name) <<
            endl;
    return {};
}
void communication::reload_config(const std::string &file)
{
    cout << "communication: reload config called for file: " <<
            (file.empty() ? "old file" : file) << endl;
}
void communication::exit()
{
    cout << "communication: exit called" << endl;
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

void communication::send(msg_hdr *msg)
{
    cout << "Sending msg type " << static_cast<int>(msg->type) << ", len " << msg->total_len << " data '" << msg->data << "'." << endl;
    socket.send(msg, msg->total_len);
}

void communication::send_str(const std::string &str, msg_type type)
{
    auto raw_msg = get_raw_msg(str.size() + 1);
    msg_hdr *msg = raw_msg.get();

    msg->type = type;
    strcpy(msg->data, str.c_str());
    send(msg);
}

void communication::rep_start(const std::string &name)
{
    try {
        master->start(name);
        send_str(name + ": started", msg_type::REP_REP);
    } catch (const exception &e) {
        send_str(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_stop(const std::string &name)
{
    try {
        master->stop(name);
        send_str(name + ": stopped", msg_type::REP_REP);
    } catch (const exception &e) {
        send_str(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_restart(const std::string &name)
{
    try {
        master->restart(name);
        send_str(name + ": restarted", msg_type::REP_REP);
    } catch (const exception &e) {
        send_str(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_status(const std::string &name)
{
    try {
        master->restart(name);
    } catch (const exception &e) {
        send_str(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_reload_config(const std::string &name)
{
    try {
        master->restart(name);
        send_str(name + ": started", msg_type::REP_REP);
    } catch (const exception &e) {
        send_str(name + ": error: " + e.what(), msg_type::REP_ERR);
    }
}

void communication::rep_exit()
{
    send_str("daemon: goodbye", msg_type::REP_REP);
    master->exit();
}

void communication::cli_get_reply()
{
    zmq::message_t reply;
    socket.recv(&reply);
    msg_hdr *msg = static_cast<msg_hdr *>(reply.data());
    if (msg->type == msg_type::REP_REP)
        cout << "daemon: " << msg->data << endl;
    else if (msg->type == msg_type::REP_ERR)
        cerr << "daemon: " << msg->data << endl;
    else
        cerr << "error: an unrecognized type reply was received" << endl;
}

#include <iostream>
#include <memory>
#include <cstring>

#include "communication.hpp"

using namespace std;

communication::communication(bool master) : context(1), socket(context, (master ? ZMQ_REP : ZMQ_REQ))
{
    if (master) {
        socket.bind((string("tcp://*:") + to_string(TDAEMON_PORT)));
    } else {
        socket.connect((string("tcp://localhost:") + to_string(TDAEMON_PORT)));
    }
}

void communication::run_master(taskmaster &/*master*/)
{
    cout << "Start master..." << endl;
    while (true) {
        zmq::message_t request;
        socket.recv(&request);
        msg_hdr *msg = static_cast<msg_hdr *>(request.data());
        cout << "Master recived message: " << msg->data << endl;
        // XXX process message and reply
    }
}

void communication::start(const std::string &name)
{
    size_t msg_len = sizeof(msg_hdr) + name.size() + 1;
    vector<char> raw_msg(msg_len);
    msg_hdr *msg = reinterpret_cast<msg_hdr *>(raw_msg.data());

    msg->type = msg_type::REQ_START;
    msg->total_len = msg_len;
    strcpy(msg->data, name.c_str());

    send(msg);
    // XXX get reply
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

void communication::send(msg_hdr *msg)
{
    cout << "Sending msg type " << static_cast<int>(msg->type) << ", len " << msg->total_len << " data '" << msg->data << "'." << endl;
    socket.send(msg, msg->total_len);
}

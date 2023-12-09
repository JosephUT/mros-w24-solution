#include "mros/node.hpp"

Node::Node(const std::string &node_name) : logger_(Logger::getLogger()), core_(MROS::getMROS()) {
    LogContext context("Node::Node");
    logger_.debug("Initializing Node");
    core_.registerHandler();

    logger_.debug("Node constructor complete");
}

Node::~Node() {
    LogContext context("~Node");
    logger_.debug("Cleaning up");

    logger_.debug("Node destructor complete");
}

bool Node::status() const {
    return core_.status();
}

void Node::spin() {
    LogContext context("Node::spin()");
    logger_.debug("spinning");

    while(status()) {
        std::this_thread::sleep_for(100ms);
    }

    logger_.debug("Exiting spin");
}

void Node::spinOnce() {
    LogContext context("Node::spinOnce()");
    logger_.debug("Spinning once");

    std::this_thread::sleep_for(100ms);

    logger_.debug("Exiting spinOnce");
}

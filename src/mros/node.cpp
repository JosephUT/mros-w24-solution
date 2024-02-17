#include "mros/node.hpp"

#include <iostream>

Node::Node(const std::string &node_name) : logger_(Logger::getLogger()), core_(MROS::getMROS()), node_name_(node_name) {
  LogContext context("Node::Node");
  // Set up the client rpc socket with the Mediator server address.
  bson_rpc_client_ = std::make_unique<ClientBsonRPCSocket>(AF_INET, "127.0.0.1", 13330);

  // Connect to the Mediator and send the node's name to start the rpc.
  json connecting_message {{"node_name", node_name}};
  bool rpc_client_connected = false;
  do {
    try {
      bson_rpc_client_->connectToServer(connecting_message);
      std::this_thread::sleep_for(100ms);
      rpc_client_connected = true;
    } catch (std::exception const& e) {
      logger_.info(e.what());
    }
  } while (!rpc_client_connected);
  // Spin off a thread to check for the two possible shutdown signals: status turning false, and the client being closed.
  shutdown_sentinel_ = std::thread(&Node::checkStatusAndHandleShutdown, this);
}

Node::~Node() {
  if (bson_rpc_client_->connected()) bson_rpc_client_->close();
  shutdown_sentinel_.join();
}

bool Node::connected() {
  return bson_rpc_client_->connected() && core_.status();
}

void Node::close() {
  bson_rpc_client_->close();
}

void Node::checkStatusAndHandleShutdown() {
  // NOTE: This could be made to use a condition variable, but it would involve refactoring the signal handler.
  while (status() && bson_rpc_client_->connected()) {
    std::this_thread::sleep_for(10ms);
  }
  if (bson_rpc_client_->connected()) bson_rpc_client_->close();
  // TODO: Close all owned publishers and subscribers.
}

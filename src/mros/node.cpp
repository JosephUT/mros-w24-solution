#include "mros/node.hpp"

Node::Node(const std::string &node_name) : logger_(Logger::getLogger()), core_(MROS::getMROS()), node_name_(node_name) {
  LogContext context("Node::Node");
  // Set up the client rpc socket with the Mediator server address.

  // Get host:port of the client rpc socket and resolve it to the node URI.

  // Make connecting json message with node_uri and node_name.

  // Connect to the Mediator and start the rpc.

  // Spin off a thread to check for the two possible shutdown signals: status turning false, and the client being closed.
  shutdown_sentinel_ = std::thread(&Node::checkStatusAndHandleShutdown, this);
}

Node::~Node() {}

void Node::close() {

}

void Node::checkStatusAndHandleShutdown() {
  // TODO: Make this just wait on a condition variable. Should be signaled by at least MROS, closing callback, and user close().
  while (status() && bson_rpc_client_->connected()) {
    std::this_thread::sleep_for(10ms);
  }
  if (bson_rpc_client_->connected()) bson_rpc_client_->close();
  // TODO: Close all owned publishers and subscribers.
}

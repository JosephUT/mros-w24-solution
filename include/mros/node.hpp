#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "logging/logging.hpp"
#include "messages/exampleMessages.hpp"
#include "mros/mros.hpp"
#include "mros/publisher.hpp"
#include "mros/subscriber.hpp"
#include "socket/bson_rpc_socket/client_bson_rpc_socket.hpp"

using namespace std::chrono_literals;

class Node : public std::enable_shared_from_this<Node> {
 public:
  /**
   * Set up the node's rpc connection to the mediator and then start the sentinel thread.
   */
  explicit Node(std::string const &node_name);

  /**
   * Set the is_shutdown flag to true and then join the sentinel thread.
   */
  ~Node();

  /**
   * Close the rpc connection. Once the rpc is closed, the shutdown sentinel will complete the shutdown sequence.
   */
  void close();

 private:
  /**
   * Get the status of the mediator.
   */
  bool status() const { return core_.status(); }

  /**
   * Check that the status is true and the rpc socket is connected.
   */
  void checkStatusAndHandleShutdown();

  std::unique_ptr<ClientBsonRPCSocket> bson_rpc_client_;

  std::thread shutdown_sentinel_;

  Logger &logger_;
  MROS &core_;
  std::string node_name_;
};

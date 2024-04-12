#pragma once

#include <arpa/inet.h>

#include <memory>

#include "logging/logging.hpp"
#include "mros/node_base.hpp"
#include "socket/bson_socket/connection_bson_socket.hpp"
#include "socket/server_socket.hpp"

class Node;

/**
 * Publisher base class for providing interface to Node.
 */
class PublisherBase {
  friend class Node;
 protected:
  PublisherBase() = default;

  virtual ~PublisherBase() = default;

  virtual void disconnect() = 0;

  virtual std::pair<std::string, int> getAddress() = 0;
};

/**
 * Publisher template class to return to user for use in messaging.
 */
template <typename MessageT>
class Publisher : public std::enable_shared_from_this<Publisher<MessageT>>, public PublisherBase {
 public:
  Publisher() = delete;

  ~Publisher() override;

  void publish(MessageT const &message);

  friend class Node;
 private:
  Publisher(std::weak_ptr<NodeBase> node, std::string topic_name);

  std::pair<std::string, int> getAddress() override;

  void disconnect() override;

  std::weak_ptr<NodeBase> node_;
  std::string topic_name_;

  std::shared_ptr<ServerSocket> subscriber_acceptor_;
  std::thread accepting_thread_;
  std::vector<std::shared_ptr<ConnectionBsonSocket>> subscriber_connections_;
  std::mutex subscriber_connections_mutex_;

  Logger &logger_;
};

template <typename MessageT>
Publisher<MessageT>::Publisher(std::weak_ptr<NodeBase> node, std::string topic_name)
    : node_(std::move(node)), topic_name_(std::move(topic_name)), logger_(Logger::getLogger()) {
  // Initialize the server socket to port zero so that the kernel will choose a valid port.
  subscriber_acceptor_ = std::make_shared<ServerSocket>(AF_INET, "127.0.0.1", 0, 100);

  // TODO: Start and detach the accepting thread to handle incoming subscriber connections.
}

template <typename MessageT>
Publisher<MessageT>::~Publisher() {
  // TODO: Let the accepting thread fall through and finish in some way.

  // TODO: Break all subscriber connections.

  // Tell the Node to remove this Publisher if the Node is available.
  if (auto const& node = node_.lock()) {
    node->removePublisherByTopic(topic_name_);
  }
}

template <typename MessageT>
void Publisher<MessageT>::disconnect() {
  // TODO: Disconnect all the subscriber connections.
  std::cout << "disconnected publisher" << std::endl;
}

template <typename MessageT>
std::pair<std::string, int> Publisher<MessageT>::getAddress() {
  return subscriber_acceptor_->getAddressPort();
}

template <typename MessageT>
void Publisher<MessageT>::publish(const MessageT &message) {
  // TODO: Send the message to all subscriber connections.
}

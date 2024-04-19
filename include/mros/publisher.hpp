#pragma once

#include <arpa/inet.h>

#include <memory>

#include "logging/logging.hpp"
#include "mros/node_base.hpp"
#include "socket/bson_socket/connection_bson_socket.hpp"
#include "socket/server_socket.hpp"

using namespace std::chrono_literals;

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
requires JsonConvertible<MessageT>
class Publisher : public std::enable_shared_from_this<Publisher<MessageT>>, public PublisherBase {
 public:
  Publisher() = delete;

  ~Publisher() override;

  void publish(MessageT message);

  friend class Node;
 private:
  Publisher(std::weak_ptr<NodeBase> node, std::string topic_name);

  std::pair<std::string, int> getAddress() override;

  void disconnect() override;

  void acceptConnectionsUntilDisconnect();

  std::weak_ptr<NodeBase> node_;
  std::string topic_name_;

  std::shared_ptr<ServerSocket> subscriber_acceptor_;
  std::thread accepting_thread_;
  std::atomic<bool> connected_;

  std::vector<std::shared_ptr<ConnectionBsonSocket>> subscriber_connections_;
  std::mutex subscriber_connections_mutex_;

  Logger &logger_;
};

template <typename MessageT>
requires JsonConvertible<MessageT>
Publisher<MessageT>::Publisher(std::weak_ptr<NodeBase> node, std::string topic_name)
    : node_(std::move(node)), topic_name_(std::move(topic_name)), connected_(true), logger_(Logger::getLogger()) {
  // Initialize the server socket to port zero so that the kernel will choose a valid port.
  subscriber_acceptor_ = std::make_shared<ServerSocket>(AF_INET, "127.0.0.1", 0, 100);

  // Start and detach the accepting thread to handle incoming subscriber connections.
  accepting_thread_ = std::thread([this]() -> void { acceptConnectionsUntilDisconnect(); });
}

template <typename MessageT>
requires JsonConvertible<MessageT>
Publisher<MessageT>::~Publisher() {
  // Trigger the shutdown sequence for the accepting thread if it has not already been triggered.
  if (connected_) connected_ = false;

  // Wait for the accepting thread to finish closing the server and connections.
  accepting_thread_.join();

  // Tell the Node to remove this Publisher if the Node is available.
  if (auto const& node = node_.lock()) {
    node->removePublisherByTopic(topic_name_);
  }
}

template <typename MessageT>
requires JsonConvertible<MessageT>
void Publisher<MessageT>::disconnect() {
  // Set connection flag to false so that the accepting thread shuts down.
  connected_ = false;
}

template <typename MessageT>
requires JsonConvertible<MessageT>
std::pair<std::string, int> Publisher<MessageT>::getAddress() {
  return subscriber_acceptor_->getAddressPort();
}

template <typename MessageT>
requires JsonConvertible<MessageT>
void Publisher<MessageT>::publish(MessageT message) {
  json json_message = message.convert_to_json();

  // Send the message to all subscriber connections. Hold the lock for the whole send cycle so that shutdown will not
  // cause messages to only be sent to some subscribers.
  subscriber_connections_mutex_.lock();

  // Send on all connections, removing the ones that throw an error.
  std::erase_if(subscriber_connections_, [json_message](std::shared_ptr<ConnectionBsonSocket> const& input) -> bool {
    try {
      input->sendMessage(json_message);
      return false;
    } catch (PeerClosedException const& e) {
      return true;
    } catch (...) {
      return true;
    }
  });
  subscriber_connections_mutex_.unlock();
}

template<typename MessageT>
requires JsonConvertible<MessageT>
void Publisher<MessageT>::acceptConnectionsUntilDisconnect() {
  while (connected_) {
    auto subscriber_connection = subscriber_acceptor_->acceptConnection<ConnectionBsonSocket>();

    // If a non-null connection was created, add it to the container of connections.
    if (subscriber_connection) {
      subscriber_connections_mutex_.lock();
      subscriber_connections_.push_back(subscriber_connection);
      subscriber_connections_mutex_.unlock();
    }
    std::this_thread::sleep_for(10ms);
  }
  // Close the server so that no more connections can be added.
  subscriber_acceptor_->close();

  // Disconnect all the subscriber connections (handled by dtor of bson socket object).
  subscriber_connections_mutex_.lock();
  subscriber_connections_.clear();
  subscriber_connections_mutex_.unlock();
}

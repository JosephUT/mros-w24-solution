#pragma once

#include "logging/logging.hpp"
#include "mros/node_base.hpp"
#include "socket/bson_socket/client_bson_socket.hpp"

class Node;

/**
 * Subscriber base class for providing interface to Node.
 */
class SubscriberBase {
  friend class Node;

 protected:
  SubscriberBase() = default;

  virtual ~SubscriberBase() = default;

  virtual void disconnect() = 0;

  virtual void connectToPublisher(std::string const& host, int port) = 0;

  virtual void spin() = 0;

  virtual void spinOnce() = 0;
};

/**
 * Subscriber template class to return to user for use in messaging.
 */
template <typename MessageT>
class Subscriber : public std::enable_shared_from_this<Subscriber<MessageT>>, public SubscriberBase {
 public:
  Subscriber() = delete;

  ~Subscriber() override;

  void spin() override;

  void spinOnce() override;

  friend class Node;

 private:
  Subscriber(std::weak_ptr<NodeBase> node, std::string topic_name, std::uint32_t queue_size,
             std::function<void(MessageT)> callback);

  void connectToPublisher(std::string const& host, int port) override;

  void disconnect() override;

  std::string topic_name_;
  std::uint32_t queue_size_;
  std::function<void(MessageT)> callback_;

  std::weak_ptr<NodeBase> node_;

  std::queue<MessageT> message_queue_;
  // TODO: container for client sockets, threads, and synchronization variables for handling spin() and spinOnce().

  Logger& logger_;
};

template <typename MessageT>
Subscriber<MessageT>::Subscriber(std::weak_ptr<NodeBase> node, std::string topic_name, std::uint32_t queue_size,
                                 std::function<void(MessageT)> callback)
    : node_(std::move(node)),
      topic_name_(std::move(topic_name)),
      queue_size_(queue_size),
      callback_(callback),
      logger_(Logger::getLogger()) {
  // TODO:
}

template <typename MessageT>
Subscriber<MessageT>::~Subscriber() {
  // TODO: Let receiving threads fall through and finish in some way, closing all publisher connections in the process.

  // Tell the Node to remove this Subscriber if the Node is available.
  if (auto const& node = node_.lock()) {
    node->removeSubscriberByTopic(topic_name_);
  }
}

template <typename MessageT>
void Subscriber<MessageT>::disconnect() {
  // TODO:
  std::cout << "disconnecting subscriber" << std::endl;
}

template <typename MessageT>
void Subscriber<MessageT>::connectToPublisher(std::string const& host, int port) {
  // TODO:
  std::cout << "connecting to publisher on " << host << ":" << port << std::endl;
}

template <typename MessageT>
void Subscriber<MessageT>::spin() {
  // TODO:
}

template <typename MessageT>
void Subscriber<MessageT>::spinOnce() {
  // TODO:
}

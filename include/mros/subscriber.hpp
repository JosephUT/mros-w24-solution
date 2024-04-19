#pragma once

#include <unordered_set>

#include "logging/logging.hpp"
#include "mros/utils/utils.hpp"
#include "mros/node_base.hpp"
#include "socket/bson_socket/client_bson_socket.hpp"

using PublisherURI = std::string;

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
requires JsonConvertible<MessageT>
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

  void receiveMessagesUntilDisconnect();

  void executeCallbacksUntilDisconnect();

  std::weak_ptr<NodeBase> node_;

  std::string topic_name_;
  std::uint32_t queue_size_;
  std::function<void(MessageT)> callback_;

  std::queue<MessageT> message_queue_;
  std::mutex message_queue_mutex_;
  std::condition_variable queue_empty_condition_variable_;

  std::unordered_map<PublisherURI, std::shared_ptr<ClientBsonMessageSocket>> publisher_connections_;
  std::mutex publisher_connections_mutex_;

  std::thread receiving_thread_;
  std::thread spinning_thread_;
  std::atomic<bool> spinning_;
  std::atomic<bool> connected_;

  Logger& logger_;
};

template <typename MessageT>
requires JsonConvertible<MessageT>
Subscriber<MessageT>::Subscriber(std::weak_ptr<NodeBase> node, std::string topic_name, std::uint32_t queue_size,
                                 std::function<void(MessageT)> callback)
    : node_(std::move(node)),
      topic_name_(std::move(topic_name)),
      queue_size_(queue_size),
      callback_(callback),
      connected_(true),
      logger_(Logger::getLogger()) {}

template <typename MessageT>
requires JsonConvertible<MessageT>
Subscriber<MessageT>::~Subscriber() {
  // Set connected to false so that the receiving and spinning threads will finish.
  connected_ = false;

  // Wait for the receiving and spinning threads to finish if they were ever started.
  if (receiving_thread_.joinable()) receiving_thread_.join();
  if (spinning_thread_.joinable()) {
    // The spinning thread may be waiting on the empty queue condition variable which will prevent it from joining.
    // To release from the wait we simply take the queue lock and add a dummy message, making the queue empty. Before
    // calling the callback in the spinning thread, we check if the Node is connected, so that this dummy message will
    // not be used to execute a user callback.
    message_queue_mutex_.lock();
    message_queue_.push(MessageT{});
    queue_empty_condition_variable_.notify_one();
    message_queue_mutex_.unlock();

    // The spinning thread exits the wait with the above unlock command and will then exit and be joined.
    spinning_thread_.join();
  }

  // Tell the Node to remove this Subscriber if the Node is available.
  if (auto const& node = node_.lock()) {
    node->removeSubscriberByTopic(topic_name_);
  }
}

template <typename MessageT>
requires JsonConvertible<MessageT>
void Subscriber<MessageT>::disconnect() {
  connected_ = false;
}

template <typename MessageT>
requires JsonConvertible<MessageT>
void Subscriber<MessageT>::connectToPublisher(std::string const& host, int port) {
  try {
    // Create a new client socket and connect it to the specified host and port.
    auto client = std::make_shared<ClientBsonMessageSocket>(AF_INET, host, port);
    client->connect();
    std::lock_guard<std::mutex> publisher_connection_mutex(publisher_connections_mutex_);
    publisher_connections_.insert({toURI(host, port), client});

    // If the receiving thread has not yet been started, start it. This will only happen on the first connection.
    if (!receiving_thread_.joinable())
      receiving_thread_ = std::thread([this]() -> void { receiveMessagesUntilDisconnect(); });
  } catch (SocketException const& e) {
    // If setting up or connecting the socket has failed, assume the publisher has closed and return silently.
  } catch (...) {
  }
}

template <typename MessageT>
requires JsonConvertible<MessageT>
void Subscriber<MessageT>::spin() {
  // Start the spinning thread and return control to the user.
  spinning_thread_ = std::thread([this]() -> void { executeCallbacksUntilDisconnect(); });
}

template <typename MessageT>
requires JsonConvertible<MessageT>
void Subscriber<MessageT>::spinOnce() {
  MessageT message;
  message_queue_mutex_.lock();

  // Get a message off the top of the queue if there is one, and use it to execute a callback.
  if (!message_queue_.empty()) {
    message = message_queue_.front();
    message_queue_.pop();
  }
  message_queue_mutex_.unlock();
  callback_(message);
}

template <typename MessageT>
requires JsonConvertible<MessageT>
void Subscriber<MessageT>::receiveMessagesUntilDisconnect() {
  std::unordered_map<PublisherURI, std::shared_ptr<ClientBsonMessageSocket>> publisher_connections_duplicate;
  std::unordered_set<PublisherURI> disconnected_publisher_uris;
  json json_message;
  MessageT message;
  while (connected_) {
    // Copy out the publisher connections for this receive cycle to avoid holding a lock while calling receive().
    publisher_connections_mutex_.lock();
    publisher_connections_duplicate = publisher_connections_;
    publisher_connections_mutex_.unlock();

    // Receive a message and add it to the message queue for each connection.
    for (const auto& uri_connection_pair : publisher_connections_duplicate) {
      try {
        // Receive the message, which will throw PeerClosedException if the publisher has disconnected.
        json_message = uri_connection_pair.second->receiveMessage();
        message.set_from_json(json_message);

        // Drop messages from the front of the queue if the queue size has been exceeded and add the new message.
        message_queue_mutex_.lock();
        while (message_queue_.size() > queue_size_ + 1) {
          message_queue_.pop();
        }
        message_queue_.push(message);

        // If the queue was empty before adding the message, signal the queue condition variable.
        if (message_queue_.size() == 1) queue_empty_condition_variable_.notify_one();
        message_queue_mutex_.unlock();

        // Add publisher connections that throw errors to the list of connections to be removed.
      } catch (PeerClosedException const& e) {
        disconnected_publisher_uris.insert(uri_connection_pair.first);
      } catch (...) {
        disconnected_publisher_uris.insert(uri_connection_pair.first);
      }
    }

    // Remove all the disconnected publishers before the next cycle.
    publisher_connections_mutex_.lock();
    for (const auto& publisher_uri : disconnected_publisher_uris) {
      publisher_connections_.erase(publisher_uri);
    }
    publisher_connections_mutex_.unlock();

    // Erase all the data from this receive cycle.
    disconnected_publisher_uris.clear();
    publisher_connections_duplicate.clear();
  }
}

template <typename MessageT>
requires JsonConvertible<MessageT>
void Subscriber<MessageT>::executeCallbacksUntilDisconnect() {
  MessageT message;
  while (connected_) {
    std::unique_lock<std::mutex> unique_message_queue_mutex(message_queue_mutex_);

    // If the queue is empty, wait for the receiving thread to populate the queue.
    if (message_queue_.empty()) {
      queue_empty_condition_variable_.wait(unique_message_queue_mutex,
                                           [this]() -> bool { return !message_queue_.empty(); });
    }

    // Get a message off the top of the queue and use it to execute a callback.
    message = message_queue_.front();
    message_queue_.pop();
    unique_message_queue_mutex.unlock();

    // Check connection to ensure this message isn't the dummy message pushed in the shutdown routine.
    if (connected_) callback_(message);
  }
}

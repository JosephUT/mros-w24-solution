#ifndef MROS_W24_SOLUTION_PUBLISHER_HPP
#define MROS_W24_SOLUTION_PUBLISHER_HPP

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_set>

#include <logging/logging.hpp>
#include <mros/mros.hpp>

#include <socket/bson_socket/connection_bson_socket.hpp>
#include <socket/server_socket.hpp>

using namespace std::chrono_literals;

class Node;

class PublisherBase {
 public:
  friend class Node;

  virtual ~PublisherBase() = default;
};

template <typename MessageT>
class Publisher : public std::enable_shared_from_this<Publisher<MessageT>>, public PublisherBase {
 public:

  Publisher() = delete;

  ~Publisher() override;

  void publish(MessageT const &msg);

  std::string getTopicName() const;

 private:
  Publisher(std::weak_ptr<Node> node, std::string topic_name, std::uint32_t queue_size);

  bool status();

  void socketListener();


  std::weak_ptr<Node> node_;
  std::string topic_name_;
  std::uint32_t queue_size_;
  Logger &logger_;
  MROS &core_;

  std::thread socketListenerThread_;

  std::shared_ptr<ServerSocket> publisherSocket_;

  std::vector<std::shared_ptr<ConnectionBsonSocket>> subscribers_;
  std::mutex subscribersMutex_;



};

template <typename MessageT>
Publisher<MessageT>::Publisher(std::weak_ptr<Node> node, std::string topic_name, std::uint32_t queue_size)
    : node_(std::move(node)),
      topic_name_(std::move(topic_name)),
      queue_size_(queue_size),
      logger_(Logger::getLogger()),
      core_(MROS::getMROS()) {
  LogContext context("Publisher::Publisher");
  logger_.debug("Initializing Publisher");
  core_.registerHandler();

  publisherSocket_ = std::make_shared<ServerSocket>(AF_INET, "127.0.0.1", 8080, 100);


  logger_.debug("Publisher constructor complete");
}

template <typename MessageT>
Publisher<MessageT>::~Publisher() {
  LogContext context("Publisher::~Publisher");
  logger_.debug("Cleaning up");

  socketListenerThread_.join();
  logger_.debug("After socketListenerThread join");

  // TODO close all sockets

  logger_.debug("Publisher destructor complete");
}

template <typename MessageT>
void Publisher<MessageT>::publish(const MessageT &msg) {
  LogContext context("Publisher::publish()");

  subscribersMutex_.lock();
  for (auto &sub : subscribers_) {
    // TODO send message to all subscribers

  }
  subscribersMutex_.unlock();

  logger_.debug("Exiting publish()");
}

template <typename MessageT>
std::string Publisher<MessageT>::getTopicName() const {
  return topic_name_;
}

template <typename MessageT>
bool Publisher<MessageT>::status() {
  return core_.status();
}

template <typename MessageT>
void Publisher<MessageT>::socketListener() {
  LogContext context("Publisher::socketListener()");
  logger_.debug("Spinning socketListener");

  while (status()) {
    std::this_thread::sleep_for(100ms);
    // Accept and handle incoming connections
  }

  logger_.debug("Exiting socketListener");
}

#endif  // MROS_W24_SOLUTION_PUBLISHER_HPP

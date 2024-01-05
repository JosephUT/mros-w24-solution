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

#include "logging/logging.hpp"
#include "mros/mros.hpp"

using namespace std::chrono_literals;

// Lets PublisherBase know Node exists without inclusion to know it exists
class Node;
// Non-templated base class for Publishers to allow for ownership by Pointer
class PublisherBase {
 public:
  friend class Node;

  virtual ~PublisherBase() = default;
};

/**
 * Publisher class
 * @tparam MessageT Message Type. Requires conversion to and from json
 */
template <typename MessageT>
class Publisher : public std::enable_shared_from_this<Publisher<MessageT>>, public PublisherBase {
 public:
  /**
   * Deleted base constructor
   */
  Publisher() = delete;

  /**
   * Publisher constructor. Please use constructor from node
   * @param node Non-owning pointer of node
   * @param topic_name String name of topic shared between publishers and subscribers
   * @param queue_size Number of messages allowed on the message queue before refusal
   */
  Publisher(std::weak_ptr<Node> node, std::string topic_name, std::uint32_t queue_size);

  /**
   * Overridden publisher destructor
   * Requirements currently unknown TODO
   */
  ~Publisher() override;

  /**
   * Pubishes message on topic defined by ctor
   * @param msg Message to be sent
   */
  void publish(MessageT const &msg);

  /**
   * Trivial accessor for Topic Name
   * @return topic name
   */
  std::string getTopicName() const;

 private:

  /**
   * MROS status accessor for Publisher
   * @return
   */
  bool status();

  /**
   * TODO
   */
  void socketListener();

  std::string topic_name_;
  std::uint32_t queue_size_;

  std::weak_ptr<Node> node_;

  std::thread socketListenerThread_;

  std::vector<int> subscribers_;
  std::mutex subscribersMutex_;

  Logger &logger_;
  MROS &core_;
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

  socketListenerThread_ = std::thread(&Publisher::socketListener, this);

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

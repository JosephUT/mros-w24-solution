#ifndef MROS_W24_SOLUTION_NODE_HPP
#define MROS_W24_SOLUTION_NODE_HPP

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

using namespace std::chrono_literals;

/**
 * Factory class for Publishers and Subscribers
 */
class Node : public std::enable_shared_from_this<Node> {
 public:

  /**
   * Base contructor for Node class
   * @param node_name String that denoted the name of Node. Must be unique to other nodes
   */
  explicit Node(std::string const &node_name);

  /**
   * Destructor for Node. Must
   * Must close threads and do closing handshake via JSON RPC
   */
  ~Node();

  /**
   * Creator of subscribers.
   * @tparam MessageT Type of message to be received. Requires conversion from bson to json to MessageT
   * @tparam CallbackT Callback to be called with a type MessageT
   * @tparam SubscriberT The type of subscriber created by by create_subscriber. We only have one subscriber type
   * @param topic_name String name of topic. Shared between publishers and subscribers
   * @param queue_size The number of messages to be allowed on the message queue before they are dismissed
   * @param callback The callback to be called
   * @return shared pointer to subscriber
   */
  template <typename MessageT, typename CallbackT = void (*)(MessageT), typename SubscriberT = Subscriber<MessageT>>
  std::shared_ptr<SubscriberT> create_subscriber(std::string topic_name, std::uint32_t queue_size,
                                                 CallbackT &&callback);

  /**
   * Creator of subscribers
   * @tparam MessageT Type of message to be received. Requires conversion from bson to json to MessageT
   * @tparam PublisherT The type of publisher created by by create_publisher. We only have one publisher type
   * @param topic_name String name of topic. Shared between publishers and subscribers
   * @param queue_size The number of messages to be allowed on the message queue before they are dismissed
   * @return shared pointer to publisher
   */
  template <typename MessageT, typename PublisherT = Publisher<MessageT>>
  std::shared_ptr<PublisherT> create_publisher(std::string topic_name, uint32_t queue_size);

  /**
   * Continuously handles incoming information such as new publishers and subscribed messages
   */
  void spin();

  /**
   * handles incoming information such as new publishers and subscribed messages once
   */
  void spinOnce();

  /**
   * MROS status accessor from node
   * @return status of MROS
   */
  bool status() const;

 private:
  /**
   * Name of node
   */
  std::string node_name_;

  /**
   * Reference to logger, invoked by Logger::getLogger()
   */
  Logger &logger_;

  /**
   * Reference to MROS
   */
  MROS &core_;

  /**
   * List of subscribers, owned in main
   */
  std::vector<std::weak_ptr<SubscriberBase>> subs_;

  /**
   * List of publishers, owned in main
   */
  std::vector<std::weak_ptr<PublisherBase>> pubs_;

  /**
   * Thread pool as needed
   */
  // std::vector<std::thread> threadPool_;
};

template <typename MessageT, typename PublisherT>
std::shared_ptr<PublisherT> Node::create_publisher(std::string topic_name, uint32_t queue_size) {
  auto temp = std::make_shared<PublisherT>(shared_from_this(), std::move(topic_name), queue_size);
  pubs_.push_back(temp);
  return temp;
}

template <typename MessageT, typename CallbackT, typename SubscriberT>
std::shared_ptr<SubscriberT> Node::create_subscriber(std::string topic_name, std::uint32_t queue_size,
                                                     CallbackT &&callback) {
  std::function<void(MessageT)> callbackFunc(callback);
  auto temp =
      std::make_shared<SubscriberT>(shared_from_this(), std::move(topic_name), queue_size, std::move(callbackFunc));
  subs_.push_back(temp);
  return temp;
}
#endif  // MROS_W24_SOLUTION_NODE_HPP

#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include "logging/logging.hpp"
#include "mros/utils/utils.hpp"
#include "mros/mros.hpp"
#include "mros/node_base.hpp"
#include "mros/publisher.hpp"
#include "mros/subscriber.hpp"
#include "socket/bson_rpc_socket/client_bson_rpc_socket.hpp"

using TopicName = std::string;

class Node : public std::enable_shared_from_this<Node>, public NodeBase {
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
   * Spin all associated publishers
   */
  void spin();

  /**
   *
   */
  void spinOnce();

  /**
   *
   */
  template <typename MessageT, typename CallbackT = void (*)(MessageT), typename SubscriberT = Subscriber<MessageT>>
  requires JsonConvertible<MessageT>
  std::shared_ptr<SubscriberT> createSubscriber(std::string topic_name, std::uint32_t queue_size, CallbackT &&callback);

  /**
   *
   */
  template <typename MessageT, typename PublisherT = Publisher<MessageT>>
  requires JsonConvertible<MessageT>
  std::shared_ptr<PublisherT> createPublisher(std::string topic_name);

 private:
  /**
   * Instruct a Subscriber to add a connection to a new Publisher on the topic, given the Publisher's address.
   * Registered as a callback for the Mediator, and called by the Mediator when another Node adds a publisher on a Topic
   * subscribed to by this Node.
   */
  void connectSubscriberToPublishers(TopicName topic_name, std::vector<std::string> hosts, std::vector<int> ports);

  /**
   * Json parsing wrapper for connectSubscriberToNewPublisher() to allow registration as callback.
   */
  void jsonConnectSubscriberToPublishersCallback(json const &json);

  /**
   * Remove a Subscriber instance on a given topic if one exists. Called on destruction of a Subscriber by a user or
   * when the associated Node is shut down. This function should remove the subscriber from the Node and notify the
   * Mediator that the Subscriber has been removed.
   */
  void removeSubscriberByTopic(TopicName topic_name) override;

  /**
   * Remove a Publisher instance on a given topic if one exists. Called on destruction of a Publisher by a user or when
   * the associated Node is shut down. This function should remove the Publisher from the Node and notify the Mediator
   * that the Publisher has been removed.
   */
  void removePublisherByTopic(TopicName topic_name) override;

  /**
   * Close the connection with the Mediator, disconnect all Publishers and Subscribers, and signal the spin() condition
   * variable to unblock any user threads that were stuck on spin().
   */
  void disconnect();

  /**
   * RPC client to communicate with the Mediator.
   */
  std::unique_ptr<ClientBsonRPCSocket> bson_rpc_client_;

  /**
   * Base pointers to all created publishers. Only one publisher can be created for a given topic name.
   */
  std::unordered_map<TopicName, std::weak_ptr<PublisherBase>> publishers_;

  /**
   * Base pointers to all created subscribers. Only one subscriber can be created for a given topic name.
   */
  std::unordered_map<TopicName, std::weak_ptr<SubscriberBase>> subscribers_;

  std::mutex spin_lock_;
  std::condition_variable spin_condition_variable_;
  std::atomic<bool> connected_;
  std::string node_name_;

  MROS &mros_;
  Logger &logger_;
};

template <typename MessageT, typename PublisherT>
requires JsonConvertible<MessageT>
std::shared_ptr<PublisherT> Node::createPublisher(std::string topic_name) {
  // Copy the topic name to avoid using string invalidated by std::move().
  std::string temp_topic_name = topic_name;

  // Create a publisher and add it to the container of publishers.
  auto raw_publisher = new PublisherT(shared_from_this(), std::move(topic_name));
  auto temp_publisher = std::shared_ptr<PublisherT>(raw_publisher);
  // TODO: Check and throw an error for multiple publishers on the same topic.
  publishers_[topic_name] = temp_publisher;

  // Get the publisher's server address for subscribers to connect to.
  std::pair<std::string, int> address_port = temp_publisher->getAddress();

  // Send a full duplex request to the mediator to connect the subscriber.
  json message{{"topic_name", temp_topic_name}, {"address", address_port.first}, {"port", address_port.second}};
  bson_rpc_client_->sendRequest("addPublisher", message);

  // Return the new publisher to the user.
  return temp_publisher;
}

template <typename MessageT, typename CallbackT, typename SubscriberT>
requires JsonConvertible<MessageT>
std::shared_ptr<SubscriberT> Node::createSubscriber(std::string topic_name, std::uint32_t queue_size,
                                                    CallbackT &&callback) {
  // Copy the topic name to avoid using string invalidated by std::move().
  std::string temp_topic_name = topic_name;

  // Create a subscriber and add it to the container of subscribers.
  std::function<void(MessageT)> callbackFunc(callback);
  auto raw_subscriber =
      new Subscriber<MessageT>(shared_from_this(), std::move(topic_name), queue_size, std::move(callbackFunc));
  auto temp_subscriber = std::shared_ptr<SubscriberT>(raw_subscriber);
  // TODO: Check and throw an error for multiple subscribers on the same topic.
  subscribers_[temp_topic_name] = temp_subscriber;

  // Send a full duplex request to the mediator to connect the subscriber to any publishers on the topic.
  json message{{"topic_name", temp_topic_name}};
  bson_rpc_client_->sendRequestAndGetResponse("addSubscriber", message, "connectSubscriberToPublishers");

  // Return the new subscriber to the user.
  return temp_subscriber;
}

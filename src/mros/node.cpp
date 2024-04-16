#include "mros/node.hpp"

#include <iostream>

Node::Node(const std::string& node_name)
    : connected_(false), node_name_(node_name), mros_(MROS::getMROS()), logger_(Logger::getLogger()) {
  LogContext context("Node::Node");
  // Set up the client rpc socket with the Mediator server address.
  bson_rpc_client_ = std::make_unique<ClientBsonRPCSocket>(AF_INET, "127.0.0.1", 13331);

  // Register the callback to allow the Mediator to connect Subscribers to Publishers.
  bson_rpc_client_->registerRequestCallback("connectSubscriberToPublishers", [this](json const& input) -> void {
    return jsonConnectSubscriberToPublishersCallback(input);
  });

  // Register the closing callback to disconnect the Node and all of its Publishers and Subscribers.
  bson_rpc_client_->registerClosingCallback([this]() -> void { disconnect(); });

  // Connect to the Mediator and send the Node's name to start the rpc.
  json connecting_message{{"node_name", node_name}};
  try {
    bson_rpc_client_->connectToServer(connecting_message);
    connected_ = true;
  } catch (std::exception const& e) {
    logger_.info(e.what());
  }

  // Register the disconnect routine as a deactivation routine for ctrl+C via MROS.
  mros_.registerDeactivateRoutine([this]() -> void { disconnect(); });
}

Node::~Node() {
  if (connected_) disconnect();
}

void Node::spin() {
  // Spin all the Subscribers.
  for (const auto& topic_subscriber_ptr : subscribers_) {
    if (auto subscriber_ptr = topic_subscriber_ptr.second.lock()) {
      subscriber_ptr->spin();
    }
  }

  // Block the user thread on a condition variable that is signaled in disconnect(). The Node can only be disconnected
  // by a ctrl+C callback handled by the MROS instance or by the rpc connection being closed by the Mediator.
  std::unique_lock<std::mutex> unique_spin_lock(spin_lock_);
  spin_condition_variable_.wait(unique_spin_lock, [this]() -> bool { return !connected_; });
}

void Node::spinOnce() {
  // Spin all the Subscribers once.
  for (const auto& topic_subscriber_ptr : subscribers_) {
    if (auto subscriber_ptr = topic_subscriber_ptr.second.lock()) {
      subscriber_ptr->spinOnce();
    }
  }
}

void Node::connectSubscriberToPublishers(TopicName topic_name, std::vector<std::string> hosts, std::vector<int> ports) {
  // If there is a subscriber on the topic, connect it to all the supplied publisher addresses.
  auto it = subscribers_.find(topic_name);
  if (it != subscribers_.end()) {
    if (auto subscriber_ptr = it->second.lock()) {
      for (int i = 0; i < hosts.size(); ++i) {
        subscriber_ptr->connectToPublisher(hosts[i], ports[i]);
      }
    }
  }
}

void Node::jsonConnectSubscriberToPublishersCallback(json const& json) {
  // Unpack the parameters in the Json message and call the main callback function with those parameters.
  std::string topic_name = json["topic_name"];
  std::vector<std::string> hosts = json["publisher_addresses"];
  std::vector<int> ports = json["publisher_ports"];
  connectSubscriberToPublishers(topic_name, hosts, ports);
}

void Node::removeSubscriberByTopic(TopicName topic_name) {
  // Remove the pointer to the subscriber from the container.
  subscribers_.erase(topic_name);

  // Tell the Mediator to remove the subscriber from its database.
  json message {{"topic_name", topic_name}};
  bson_rpc_client_->sendRequest("removeSubscriber", message);
}

void Node::removePublisherByTopic(TopicName topic_name) {
  // Remove the pointer to the publisher from the container.
  publishers_.erase(topic_name);

  // Tell the Mediator to remove the publisher from its database.
  json message {{"topic_name", topic_name}};
  bson_rpc_client_->sendRequest("removePublisher", message);
}

void Node::disconnect() {
  // Only perform routine if Node is connected to avoid duplicate action via ctrl+C calling the closing callback.
  std::cout << "entering node disconnect" << std::endl;
  if (connected_) {
    std::cout << "setting connected to false" << std::endl;
    // Mark this Node as disconnected.
    connected_ = false;
    std::cout << "successfully set connected to false" << std::endl;

    // Close the rpc connection.
    std::cout << "before checking connection" << std::endl;
    if (bson_rpc_client_->connected()) {
      std::cout << "sending closing message" << std::endl;
      bson_rpc_client_->close();
    }
    std::cout << "after close" << std::endl;

    // Disconnect all publishers.
    for (const auto& topic_publisher : publishers_) {
      if (auto publisher_ptr = topic_publisher.second.lock()) {
        publisher_ptr->disconnect();
      }
    }

    // Disconnect all subscribers.
    std::cout << "about to iterate through subscribers" << std::endl;
    for (const auto& topic_subscriber : subscribers_) {
      if (auto subscriber_ptr = topic_subscriber.second.lock()) {
        std::cout << "disconnecting a subscriber" << std::endl;
        subscriber_ptr->disconnect();
      }
    }
    std::cout << "finished iterating through subscribers" << std::endl;

    // Signal the condition variable to release any user thread blocked on spin().
    spin_condition_variable_.notify_all();
  }
}

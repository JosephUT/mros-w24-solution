#include "mediator/mediator.hpp"

#include <iostream>

// Mediator::Mediator() : Mediator("127.0.0.1", 13330) {}

Mediator::Mediator(std::string address, int port)
    : address_(std::move(address)), port_(port), mros_(MROS::getMROS()), logger_(Logger::getLogger()) {
  // Initialize the server and begin accepting connections.
  LogContext context("Mediator::Mediator");
  try {
    bson_rpc_server_ = std::make_unique<ServerSocket>(AF_INET, address_, port_, 100);
    logger_.debug("Made server");
    handleRPCConnections();
  } catch (std::exception const &e){
    logger_.info(e.what());
    logger_.info("Mroscore is already running. Terminating");
  }
}

Mediator::~Mediator() {}

void Mediator::handleRPCConnections() {
  LogContext context("Mediator::handleRPCConnections");
  // Check that the mediator has not been killed.
  while (status()) {
    // Attempt to accept a new connection, which is non-blocking.
    auto connection_socket = bson_rpc_server_->acceptConnection<ConnectionBsonRPCSocket>();

    // If a connection was accepted, set up the connection.
    if (connection_socket) {
      // Get the address and port of the connecting client and resolve it to the node's URI.
      auto client_address_port = bson_rpc_server_->getLastClientAddressPort();
      std::string node_uri = toURI(client_address_port.first, client_address_port.second);
      node_uris_.push_back(node_uri);

      // Register the node's connection in the lookup table.
      node_table_mutex_.lock();
      node_table_[node_uri].connection = connection_socket;
      node_table_mutex_.unlock();

      // Register addNode() as the connecting callback and cache the socket pointer so that addNode() can access it.
      connection_socket->registerConnectingCallback([this, node_uri](Json const &input) -> void {
        json input_with_uri = input;
        input_with_uri["node_uri"] = node_uri;
        return jsonAddNodeCallback(input_with_uri);
      });

      // Register an addPublisher callback that automatically inserts this node's URI.
      connection_socket->registerRequestCallback("addPublisher", [this, node_uri](json const &input) -> void {
        json input_with_uri = input;
        input_with_uri["node_uri"] = node_uri;
        jsonAddPublisherCallback(input_with_uri);
      });

      // Register an addSubscriber callback that automatically inserts this node's URI.
      connection_socket->registerRequestResponseCallback("addSubscriber", [this, node_uri](json const &input) -> json {
        json input_with_uri = input;
        input_with_uri["node_uri"] = node_uri;
        return jsonAddSubscriberCallback(input_with_uri);
      });

      // Register a removeNode closing callback that automatically inserts this node's URI.
      connection_socket->registerClosingCallback([this, node_uri]() -> void { removeNode(node_uri); });

      // Call addNode() internally to set up the node data and node specific callbacks, then start the connection.
      connection_socket->startConnection();
    }
    std::this_thread::sleep_for(10ms);
  }
  // Close all connections and clear all data.
  bson_rpc_server_->close();
  topic_table_mutex_.lock();
  // Implicitly close connection sockets by clearing table.
  topic_table_.clear();
  topic_table_mutex_.unlock();
  node_table_mutex_.lock();
  node_table_.clear();
  node_table_mutex_.unlock();
  logger_.info("Mediator closed");
}

void Mediator::addNode(const NodeURI &node_uri, const std::string &node_name) {
  // Update the node table with the node's name. The connection should already be registered for this uri.
  std::lock_guard<std::mutex> node_table_guard(node_table_mutex_);
  node_table_[node_uri].name = node_name;
  std::string info = "Added Node " + node_name + " at " + node_uri;
  logger_.info(info);
}

void Mediator::addPublisher(const NodeURI &node_uri, const TopicName &topic_name, const AddressPort &address_port) {
  // Update topic table.
  topic_table_mutex_.lock();
  topic_table_[topic_name].publishing_nodes.insert(node_uri);
  std::vector<NodeURI> subscriber_uris(topic_table_[topic_name].subscribing_nodes.begin(),
                                       topic_table_[topic_name].subscribing_nodes.end());
  topic_table_mutex_.unlock();

  // Update node table and notify subscribing nodes of new publisher.
  node_table_mutex_.lock();
  node_table_[node_uri].publisher_data_by_topic[topic_name].push_back(address_port);
  for (auto const &subscriber_uri : subscriber_uris) {
    node_table_[subscriber_uri].connection->sendRequest("connectToPublishers",
                                                        {{"topic", topic_name},
                                                         {"publisher_addresses", {address_port.host}},
                                                         {"publisher_ports", {address_port.port}}});
  }
  node_table_mutex_.unlock();
  logger_.info("Added Publisher");
}

Json Mediator::addSubscriber(const NodeURI &node_uri, const TopicName &topic_name) {
  // Update node table.
  node_table_mutex_.lock();
  node_table_[node_uri].subscriber_data_by_topic[topic_name]++;
  node_table_mutex_.unlock();

  // Update topic table and get list of publishers to connect to.
  topic_table_mutex_.lock();
  topic_table_[topic_name].subscribing_nodes.insert(node_uri);
  std::vector<AddressPort> publisher_addresses(topic_table_[topic_name].publishing_nodes.begin(),
                                               topic_table_[topic_name].publishing_nodes.end());
  topic_table_mutex_.unlock();

  // Encode request for this node to subscribe to the existing publishers.
  std::vector<std::string> addresses;
  std::vector<int> ports;
  addresses.reserve(publisher_addresses.size());
  ports.reserve(publisher_addresses.size());
  for (const auto &address_port : publisher_addresses) {
    addresses.emplace_back(address_port.host);
    ports.emplace_back(address_port.port);
  }
  logger_.info("Added Subscriber");
  return json{{"topic", topic_name}, {"publisher_addresses", addresses}, {"publisher_ports", ports}};
}

void Mediator::removeNode(const NodeURI &node_uri) {
  // For each topic that the node publishes to, get the TopicName and remove this node as a publishing node in
  // topic_table_.
  topic_table_mutex_.lock();
  for (const auto &topic_pub_data_it : node_table_[node_uri].publisher_data_by_topic) {
    topic_table_[topic_pub_data_it.first].publishing_nodes.erase(node_uri);
  }

  // For each topic that the node subscribes to, get the TopicName and remove this node as a subscribing node in
  // topic_table_.
  for (const auto &topic_sub_data_it : node_table_[node_uri].subscriber_data_by_topic) {
    topic_table_[topic_sub_data_it.first].subscribing_nodes.erase(node_uri);
  }
  topic_table_mutex_.unlock();

  // Close the node's connection and remove the node data from node_table_.
  node_table_mutex_.lock();
  std::string info = "Removed Node " + node_table_[node_uri].name + " at " + node_uri;
  node_table_[node_uri].connection->close();
  node_table_.erase(node_uri);
  node_table_mutex_.unlock();
  logger_.info(info);
}

void Mediator::jsonAddNodeCallback(Json const &json) {
  NodeURI node_uri = json["node_uri"];
  std::string node_name = json["node_name"];
  addNode(node_uri, node_name);
}

void Mediator::jsonAddPublisherCallback(Json const &json) {
  NodeURI node_uri = json["node_uri"];
  TopicName topic_name = json["topic_name"];
  std::string address = json["address"];
  int port = json["port"];
  addPublisher(node_uri, topic_name, {address, port});
}

Json Mediator::jsonAddSubscriberCallback(Json const &json) {
  NodeURI node_uri = json["node_uri"];
  TopicName topic_name = json["topic_name"];
  return addSubscriber(node_uri, topic_name);
}

void Mediator::jsonRemoveNodeCallback(Json const &json) {
  NodeURI node_uri = json["node_uri"];
  removeNode(node_uri);
}

TopicData Mediator::getTopicData(const TopicName &topic_name) {
  std::lock_guard<std::mutex> topic_table_guard(topic_table_mutex_);
  return topic_table_[topic_name];
}

NodeData Mediator::getNodeData(const NodeURI &node_uri) {
  std::lock_guard<std::mutex> node_table_guard(node_table_mutex_);
  return node_table_[node_uri];
}

bool Mediator::status() {
  return mros_.status();
}

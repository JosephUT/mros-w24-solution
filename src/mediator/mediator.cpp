#include "mediator/mediator.hpp"

Mediator::Mediator() : Mediator("127.0.0.1", 13330) {}

Mediator::Mediator(std::string address, int port)
    : address_(std::move(address)), port_(port), mros_(MROS::getMROS()), logger_(Logger::getLogger()) {
  // Initialize the server and begin accepting connections.
  bson_rpc_server_ = std::make_unique<ServerSocket>(AF_INET, address_, port_, 100);
  handleRPCConnections();
}

Mediator::~Mediator() {}

void Mediator::handleRPCConnections() {
  LogContext context("Mediator::handleRPCConnections");
  // Check that the mediator has not been killed.
  while (status()) {
    // Attempt to accept a new connection, which is non-blocking.
    // TODO: acceptConnection can throw. Catch everything, log or ignore.
    auto connection_socket = bson_rpc_server_->acceptConnection<ConnectionBsonRPCSocket>();

    // If a connection was accepted, set up the connection.
    if (connection_socket) {
      // Register addNode() as the connecting callback and cache the socket pointer so that addNode() can access it.
      connection_socket->registerConnectingCallback(
          [this](Json const &json) -> void { return jsonAddNodeCallback(json); });
      pending_connection_ = connection_socket;

      // Call addNode() internally to set up the node data and node specific callbacks, then start the connection.
      connection_socket->startConnection();
    }
  }
  // Close the rpc server. Now no new connections can be set up.
  bson_rpc_server_->close();
  node_table_mutex_.lock();

  // Close the rpc connections to each node. Closing the rpc invokes the shutdown routine of each node via closing
  // callback.
  for (auto const &uri_node_data_it : node_table_) {
    uri_node_data_it.second.connection->close();
  }
  node_table_.clear();
  node_table_mutex_.unlock();
  topic_table_mutex_.lock();
  topic_table_.clear();
  topic_table_mutex_.unlock();
}

void Mediator::addNode(const NodeURI &node_uri, const std::string node_name) {
  LogContext context("Mediator::addNode");
  // Update the node table using pending_socket_ as the connection socket, since this is only set by the main thread.
  node_table_mutex_.lock();
  node_table_[node_uri] = NodeData{node_name, pending_connection_, {}, {}};
  node_table_mutex_.unlock();

  // Register an addPublisher callback that automatically inserts this node's URI.
  pending_connection_->registerRequestCallback("addPublisher", [this, node_uri](json const &input) -> void {
    json input_with_uri = input;
    input_with_uri["node_uri"] = node_uri;
    jsonAddPublisherCallback(input_with_uri);
  });

  // Register an addSubscriber callback that automatically inserts this node's URI.
  pending_connection_->registerRequestResponseCallback("addSubscriber", [this, node_uri](json const &input) -> json {
    json input_with_uri = input;
    input_with_uri["node_uri"] = node_uri;
    return jsonAddSubscriberCallback(input_with_uri);
  });

  // Register a removeNode closing callback that automatically inserts this node's URI.
  pending_connection_->registerClosingCallback([this, node_uri]() -> void { removeNode(node_uri); });
  logger_.info("Added Node");
}

void Mediator::addPublisher(const NodeURI &node_uri, const TopicName &topic_name, const AddressPort &address_port) {
  LogContext context("Mediator::addPublisher");
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
  LogContext context("Mediator::addSubscriber");
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
  LogContext context("Mediator::removeNode");
  topic_table_mutex_.lock();

  // For each topic that the node publishes to, get the TopicName and remove this node as a publishing node in
  // topic_table_.
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
  node_table_[node_uri].connection->close();
  node_table_.erase(node_uri);
  node_table_mutex_.unlock();
  logger_.info("Removed Node");
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

const TopicData Mediator::getTopicData(const TopicName &topic_name) {
  std::lock_guard<std::mutex> topic_table_guard(topic_table_mutex_);
  return topic_table_[topic_name];
}

const NodeData Mediator::getNodeData(const NodeURI &node_uri) {
  std::lock_guard<std::mutex> node_table_guard(node_table_mutex_);
  return node_table_[node_uri];
}

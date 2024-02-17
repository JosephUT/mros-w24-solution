#pragma once

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <nlohmann/json.hpp>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "logging/logging.hpp"
#include "mros/mros.hpp"
#include "mros/utils/utils.hpp"
#include "socket/bson_rpc_socket/connection_bson_rpc_socket.hpp"
#include "socket/server_socket.hpp"

using namespace std::chrono_literals;
using Json = nlohmann::json;

using TopicName = std::string;
using NodeURI = std::string;
using NumSubscribers = int;

struct AddressPort {
  std::string host;
  int port;
};

struct TopicData {
  std::unordered_set<NodeURI> publishing_nodes;
  std::unordered_set<NodeURI> subscribing_nodes;
};

struct NodeData {
  std::string name;
  std::shared_ptr<ConnectionBsonRPCSocket> connection;
  std::unordered_map<TopicName, std::vector<AddressPort>> publisher_data_by_topic;
  std::unordered_map<TopicName, NumSubscribers> subscriber_data_by_topic;
};

class Mediator : public std::enable_shared_from_this<Mediator> {
 public:
  /**
   * Set up mediator to accept connections at a specific address and port.
   */
  Mediator(std::string address = "127.0.0.1", int port = 13330);

  // Mediator();
  ~Mediator();

  /**
   * Accept and set up connections from nodes until the process is terminated, shutdown all connections on termination.
   */
  void handleRPCConnections();

  /**
   * Check whether a termination signal has been sent to the process.
   */
  bool status();

  /** Callback functions **/

  /**
   * Update node_table_ to add node. Use the pending_connection_ as the pointer to the connection socket for this node.
   * Nodes request this callback immediately after their connection is accepted.
   */
  void addNode(const NodeURI &node_uri, const std::string &node_name);

  /**
   * Update tables to add publisher. Requests that all subscribing nodes connect to the new publisher. Nodes request
   * this callback when the user creates a publisher.
   */
  void addPublisher(const NodeURI &node_uri, const TopicName &topic_name, const AddressPort &address_port);

  /**
   * Update tables to add subscriber. Requests that the calling node connect to all the existing publishers. Nodes
   * request this callback when the user creates a subscriber.
   */
  Json addSubscriber(const NodeURI &node_uri, const TopicName &topic_name);

  /**
   * Update tables to remove the node, including all of its publishers and subscribers. Close the rpc connection to that
   * node. Removal of connections between publishers and subscribers is handled by Nodes internally as
   * BsonMessageSockets throw PeerClosedException when a peer node closes its connections on shutdown. Called by Nodes
   * when they are terminated locally via closing callback, by the Mediator when it is terminated, or potentially by
   * other nodes.
   */
  void removeNode(const NodeURI &node_uri);

  /** Json decode function wrappers **/

  /**
   * Json parsing wrapper for addNode() to allow registration as a callback.
   */
  void jsonAddNodeCallback(Json const &json);

  /**
   * Json parsing wrapper for addPublisher() to allow registration as a callback.
   */
  void jsonAddPublisherCallback(Json const &json);

  /**
   * Json parsing wrapper for addSubscriber() to allow registration as a callback.
   */
  Json jsonAddSubscriberCallback(Json const &json);

  /**
   * Json parsing wrapper for removeNode() to allow registration as a callback.
   */
  void jsonRemoveNodeCallback(Json const &json);

  TopicData getTopicData(const TopicName &topic_name);
  NodeData getNodeData(const NodeURI &node_uri);

  std::unordered_map<TopicName, TopicData> topic_table_;
  std::unordered_map<NodeURI, NodeData> node_table_;
  std::mutex topic_table_mutex_;
  std::mutex node_table_mutex_;

  std::vector<std::string> node_uris_;

  std::unique_ptr<ServerSocket> bson_rpc_server_;

  std::string address_;
  int port_;
  MROS &mros_;
  Logger &logger_;
};

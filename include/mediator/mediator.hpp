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
  std::vector<NodeURI> publishing_nodes;
  std::vector<NodeURI> subscribing_nodes;
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

  Mediator();
  ~Mediator();

  /**
   * Accept and set up connections from nodes until the process is terminated, shutdown all connections on termination.
   */
  void handleRPCConnections();

  /**
   * Check whether a termination signal has been sent to the process.
   */
  bool status() { return mros_.status(); }

  /** Callback functions **/

  /**
   * Update node_table_ to add node. Nodes request this callback immediately after their connection is accepted.
   */
  void addNode();

  /**
   * Update tables to add publisher. Requests that all subscribing nodes connect to the new publisher. Nodes request
   * this callback when the user creates a publisher.
   */
  void addPublisher();

  /**
   * Update tables to add subscriber. Requests that the calling node connect to all the existing publishers. Nodes
   * request this callback when the user creates a subscriber.
   */
  Json addSubscriber();

  /**
   * Update tables to remove the node, including all of its publishers and subscribers. Close the rpc connection to that
   * node. Removal of connections between publishers and subscribers is handled by Nodes internally as
   * BsonMessageSockets throw PeerClosedException when a peer node closes its connections on shutdown. Called by Nodes
   * when they are terminated locally via closing callback, by the Mediator when it is terminated, or potentially by
   * other nodes.
   */
  void removeNode();

  /** Request functions **/

  /**
   * Request that a node connect all of its subscribers of a specified topic to a list of publisher address:port pairs.
   * The receiving node handles adding these additional connections and ignores address:port pairs to which a given
   * subscriber is already connected. Called from within addPublisher() and addSubscriber().
   */
  void connectNodeToPublishers();

  /** Json decode function wrappers **/

  /**
   * Json parsing wrapper for addPublisher() to allow registration.
   */
  void jsonAddNodeCallback(Json const &json);
  void jsonAddPublisherCallback(Json const &json);
  Json jsonAddSubscriberCallback(Json const &json);
  void jsonRemoveNodeCallback(Json const &json);
  void jsonConnectNodeToPublishers(Json const &json);

  const TopicData getTopicData(const TopicName& topic_name);
  const NodeData getNodeData(const NodeURI& node_uri);

  std::unordered_map<TopicName, TopicData> topic_table_;
  std::unordered_map<NodeURI, NodeData> node_table_;
  std::mutex topic_table_mutex_;
  std::mutex node_table_mutex_;

  std::string address_;
  int port_;
  MROS &mros_;
  Logger &logger_;
  std::unique_ptr<ServerSocket> bson_rpc_server_;
};

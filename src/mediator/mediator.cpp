#include "mediator/mediator.hpp"

//---------------------------------------------Mediator--------------------------------------

Mediator::Mediator() : Mediator("127.0.0.1", 13330) {}

Mediator::Mediator(std::string address, int port)
    : address_(std::move(address)), port_(port), mros_(MROS::getMROS()), logger_(Logger::getLogger()) {
  LogContext context("Mediator::Mediator");

  // Initialize the server and begin accepting connections.
  bson_rpc_server_ = std::make_unique<ServerSocket>(AF_INET, address_, port_, 100);
  logger_.debug("Mediator initialized");
  handleRPCConnections();
}

Mediator::~Mediator() {}

void Mediator::handleRPCConnections() {
  LogContext context("Mediator::RPCListenerThread");
  logger_.debug("Starting RPCListenerThread");

  // Check that the mediator has not been killed.
  while (status()) {
    // Attempt to accept a new connection, which is non-blocking.
    // TODO: acceptConnection can throw. Catch everything, log or ignore.
    auto connection_socket = bson_rpc_server_->acceptConnection<ConnectionBsonRPCSocket>();

    // If a connection was accepted, set up the connection.
    if (connection_socket) {
      // Register all necessary callbacks.
//      connection_socket->registerRequestResponseCallback(
//          "subscribe", [this](Json const &json1) -> Json { return jsonSubscribeCallback(json1); });
//      connection_socket->registerRequestCallback(
//          "unsubscribe", [this](Json const &json1) -> Json { return jsonUnsubscribeCallback(json1); });
//      connection_socket->registerRequestCallback(
//          "publish", [this](Json const &json1) -> Json { return jsonPublishCallback(json1); });
//      connection_socket->registerRequestCallback(
//          "unpublish", [this](Json const &json1) -> Json { return jsonUnpublishCallback(json1); });

      // Store a shared_ptr to the node.

      // Start the rpc connection with the connecting node.
      connection_socket->startConnection();
    }
  }
  logger_.debug("Exiting RPCListenerThread");

  // TODO: handle shutdown logic here.
  // Close the rpc server. Now no new connections can be set up.
  // Close the rpc connections to each node. Closing the rpc invokes the shutdown routine of each node via closing callback.
}


/**
   * Update node_table_ to add node. Nodes request this callback immediately after their connection is accepted.
 */
void Mediator::addNode() {}

/**
   * Update tables to add publisher. Requests that all subscribing nodes connect to the new publisher. Nodes request
   * this callback when the user creates a publisher.
 */
void addPublisher() {}

/**
   * Update tables to add subscriber. Requests that the calling node connect to all the existing publishers. Nodes
   * request this callback when the user creates a subscriber.
 */
Json addSubscriber() { return Json(); }

/**
   * Update tables to remove the node, including all of its publishers and subscribers. Close the rpc connection to that
   * node. Removal of connections between publishers and subscribers is handled by Nodes internally as
   * BsonMessageSockets throw PeerClosedException when a peer node closes its connections on shutdown. Called by Nodes
   * when they are terminated locally via closing callback, by the Mediator when it is terminated, or potentially by
   * other nodes.
 */
void removeNode() {}

/** Request functions **/

/**
   * Request that a node connect all of its subscribers of a specified topic to a list of publisher address:port pairs.
   * The receiving node handles adding these additional connections and ignores address:port pairs to which a given
   * subscriber is already connected. Called from within addPublisher() and addSubscriber().
 */
void connectNodeToPublishers() {}

/** Json decode function wrappers **/

/**
   * Json parsing wrapper for addPublisher() to allow registration.
 */
void jsonAddNodeCallback(Json const &json) {}
void jsonAddPublisherCallback(Json const &json) {}
Json jsonAddSubscriberCallback(Json const &json) { return Json(); }
void jsonRemoveNodeCallback(Json const &json) {}
void jsonConnectNodeToPublishers(Json const &json) {}

const TopicData getTopicData(const TopicName& topic_name) { return TopicData(); }
const NodeData getNodeData(const NodeURI& node_uri) { return NodeData(); }

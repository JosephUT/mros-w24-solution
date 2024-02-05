#pragma once

#include <optional>

#include "socket/bson_rpc_socket/bson_rpc_socket.hpp"
#include "socket/connection_socket.hpp"

/**
 * Class combining string RPC functionality with connection socket style initialization.
 */
class ConnectionBsonRPCSocket : virtual public ConnectionSocket, virtual public BsonRPCSocket {
 public:
  /**
   * Deleted default constructor.
   */
  ConnectionBsonRPCSocket() = delete;

  /**
   * Constructor taking a file descriptor to be called internally to acceptConnection() in ServerSockets.
   * @param file_descriptor The file descriptor to initialize the connection socket with.
   */
  explicit ConnectionBsonRPCSocket(int file_descriptor);

  /**
   * Close the socket file descriptor.
   */
  ~ConnectionBsonRPCSocket() override = default;

  /**
   * Set up a callback to be called with the connection_message from connectToServer() inside of startConnection().
   */
  void registerConnectingCallback(const RequestCallbackJson &callback);

  /**
   * Send the connecting message to the client socket to unblock it's connect() call.
   */
  void startConnection();

 private:
  /**
   * Connecting callback to be called during startConnection() if it exists.
   */
  std::optional<RequestCallbackJson> connecting_callback_;
};

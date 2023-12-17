#ifndef MROS_W24_SOLUTION_CONNECTION_JSON_RPC_SOCKET_HPP
#define MROS_W24_SOLUTION_CONNECTION_JSON_RPC_SOCKET_HPP

#include <socket/connection_socket.hpp>
#include <socket/json_rpc_socket/json_rpc_socket.hpp>

/**
 * Class combining string RPC functionality with connection socket style initialization.
 */
class ConnectionJsonRPCSocket : virtual public ConnectionSocket, virtual public JsonRPCSocket {
 public:
  /**
   * Deleted default constructor.
   */
  ConnectionJsonRPCSocket() = delete;

  /**
   * Constructor taking a file descriptor to be called internally to acceptConnection() in ServerSockets.
   * @param file_descriptor The file descriptor to initialize the connection socket with.
   */
  explicit ConnectionJsonRPCSocket(int file_descriptor);

  /**
   * Close the socket file descriptor.
   */
  ~ConnectionJsonRPCSocket() override = default;

  /**
   * Send the connecting message to the client socket to unblock it's connect() call.
   */
  void startConnection();
};

#endif //MROS_W24_SOLUTION_CONNECTION_JSON_RPC_SOCKET_HPP

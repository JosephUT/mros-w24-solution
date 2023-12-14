#ifndef MROS_W24_SOLUTION_CONNECTION_JSON_RPC_SOCKET_HPP
#define MROS_W24_SOLUTION_CONNECTION_JSON_RPC_SOCKET_HPP

#include <socket/connection_socket.hpp>
#include <socket/json_rpc_socket/json_rpc_socket.hpp>

/**
 * Class combining string RPC functionality with connection socket style initialization.
 */
class JsonConnectionRPCSocket : virtual public ConnectionSocket, virtual public JsonRPCSocket {
 public:
  /**
   * Deleted default constructor.
   */
  JsonConnectionRPCSocket() = delete;

  /**
   * Constructor taking a file descriptor to be called internally to acceptConnection() in ServerSockets.
   * @param file_descriptor The file descriptor to initialize the connection socket with.
   */
  explicit JsonConnectionRPCSocket(int file_descriptor);

  /**
   * Close the socket file descriptor.
   */
  ~JsonConnectionRPCSocket() override = default;

  /**
   * Send the connecting message to the client socket to unblock it's connect() call.
   */
  void startConnection();
};

#endif //MROS_W24_SOLUTION_CONNECTION_JSON_RPC_SOCKET_HPP

#ifndef MROS_CONNECTION_RPC_SOCKET_HPP
#define MROS_CONNECTION_RPC_SOCKET_HPP

#include <socket/connection_socket.hpp>
#include <socket/rpc_socket/rpc_socket.hpp>

/**
 * Class combining string RPC functionality with connection socket style initialization.
 */
class ConnectionRPCSocket : virtual public ConnectionSocket, virtual public RPCSocket {
 public:
  /**
   * Deleted default constructor.
   */
  ConnectionRPCSocket() = delete;

  /**
   * Constructor taking a file descriptor to be called internally to acceptConnection() in ServerSockets.
   * @param file_descriptor The file descriptor to initialize the connection socket with.
   */
  explicit ConnectionRPCSocket(int file_descriptor);

  /**
   * Close the socket file descriptor.
   */
  ~ConnectionRPCSocket() override = default;

  /**
   * Send the connecting message to the client socket to unblock it's connect() call.
   */
  void startConnection();
};

#endif  // MROS_CONNECTION_RPC_SOCKET_HPP

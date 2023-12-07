#ifndef MROS_CONNECTION_MESSAGE_SOCKET_HPP
#define MROS_CONNECTION_MESSAGE_SOCKET_HPP

#include <unistd.h>

#include <socket/connection_socket.hpp>
#include <socket/message_socket/message_socket.hpp>

/**
 * Socket class combining string messaging functionality with connection socket style construction.
 */
class ConnectionMessageSocket : virtual public ConnectionSocket, virtual public MessageSocket {
 public:
  /**
   * Deleted default constructor to disallow construction by the public.
   */
  ConnectionMessageSocket() = delete;

  /**
   * Constructor taking a file descriptor to be called internally to acceptConnection() in ServerSockets.
   * @param file_descriptor The file descriptor to initialize the connection socket with.
   */
  explicit ConnectionMessageSocket(int file_descriptor);

  /**
   * Close the socket file descriptor.
   */
  ~ConnectionMessageSocket() override = default;
};

#endif  // MROS_CONNECTION_MESSAGE_SOCKET_HPP

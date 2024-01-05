#ifndef MROS_W24_SOLUTION_CONNECTION_SOCKET_HPP
#define MROS_W24_SOLUTION_CONNECTION_SOCKET_HPP

#include <socket/socket.hpp>

/**
 *  Abstract base class for "connection sockets" which are produced by server sockets calling accept().
 */
class ConnectionSocket : virtual public Socket {
 public:
  /**
   * Deleted default constructor to disallow construction by the public.
   */
  ConnectionSocket() = delete;

  /**
   * Constructor taking a file descriptor to be called internally to acceptConnection() in ServerSockets.
   * @param file_descriptor
   */
  explicit ConnectionSocket(int file_descriptor);

  /**
   * Pure virtual destructor to force subclassing.
   */
  ~ConnectionSocket() override = 0;
};

#endif  // MROS_W24_SOLUTION_CONNECTION_SOCKET_HPP

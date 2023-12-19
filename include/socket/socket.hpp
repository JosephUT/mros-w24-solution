#ifndef MROS_W24_SOLUTION_SOCKET_HPP
#define MROS_W24_SOLUTION_SOCKET_HPP

#include <logging/logging.hpp>
/**
 * Abstract base class for socket.
 */
class Socket {
 protected:
  /**
   * Default constructor.
   */
  Socket();

  /**
   * Private constructor to be called by connection sockets.
   */
  explicit Socket(int file_descriptor);

  /**
   * Pure virtual destructor to make class abstract.
   */
  virtual ~Socket() = 0;

  /**
   * File descriptor for the socket.
   */
  int file_descriptor_;

  Logger& logger_;
};

#endif  // MROS_W24_SOLUTION_SOCKET_HPP

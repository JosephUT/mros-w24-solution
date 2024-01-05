#ifndef MROS_W24_SOLUTION_MESSAGE_SOCKET_HPP
#define MROS_W24_SOLUTION_MESSAGE_SOCKET_HPP

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <queue>
#include <socket/socket.hpp>
#include <socket/utils/peer_closed_exception.hpp>
#include <socket/utils/socket_errno_exception.hpp>
#include <socket/utils/socket_exception.hpp>
#include <sstream>
#include <string>

/**
 * Abstract mixin class to provide framed string messages over SOCK_STREAM connections. Implementation is not
 * thread safe for sending or receiving but one sending thread and one receiving thread may operate concurrently.
 */
class MessageSocket : virtual public Socket {
 public:
  /**
   * Default constructor.
   */
  MessageSocket() = default;

  /**
   * Abstract destructor to force subclassing. Closes the socket if it is not already closed.
   */
  ~MessageSocket() override = 0;

  /**
   * Send a string message in completion by calling send() until all bytes are sent.
   * @param message The string message to send.
   * @throws SocketException Throws exception on failure of send() or if this socket is closed, or if message contains a
   * delimiting character.
   * @throws PeerClosedException Throws exception if peer has closed. Users may catch and instantiate a closing
   * sequence.
   */
  void sendMessage(std::string message);

  /**
   * Receive a string message, calling recv() to fill message queue if necessary.
   * @return The string message at the top of the message queue.
   * @throws SocketException Throws exception on failure of recv() or if this socket it closed.
   * @throws PeerClosedException Throws exception if peer has closed. Users may catch and instantiate a closing
   * sequence.
   */
  std::string receiveMessage();

  /**
   * Close the socket. NOTE: close() should always be called by the user before destruction.
   */
  virtual void close();

 protected:
  /**
   * Boolean, true if socket file descriptor is open, false otherwise. Defaults to true since derived classes will be
   * open upon successful construction.
   */
  bool is_open_ = true;

 private:
  /**
   * Message queue to hold extra messages from last recv() call in ReceiveMessage().
   */
  std::queue<std::string> message_queue_;

  /**
   * Boolean that is true if the last recv() call yielded a string ending in the delimiting character, false otherwise.
   */
  bool back_is_complete_message_ = false;

  /**
   * The size of receive buffer in bytes.
   */
  static constexpr const int kReceiveBufferSize_ = 4;

  /**
   * The character that will be used to determine message boundaries.
   */
  static constexpr const char kDelimitingCharacter_ = '\n';
};

#endif  // MROS_W24_SOLUTION_MESSAGE_SOCKET_HPP

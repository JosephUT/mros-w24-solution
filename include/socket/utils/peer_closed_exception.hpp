#ifndef MROS_PEER_CLOSED_EXCEPTION_HPP
#define MROS_PEER_CLOSED_EXCEPTION_HPP

#include <socket/utils/socket_exception.hpp>

/**
 * Runtime error to throw on send() or recv() calling functions when a connected socket's peer has closed.
 */
class PeerClosedException : public SocketException {
 public:
  /**
   * Constructor feeding peer closed message indirectly to the grandparent std::runtime_exception class.
   */
  explicit PeerClosedException() : SocketException("Peer socket has closed.") {}
};

#endif  // MROS_PEER_CLOSED_EXCEPTION_HPP

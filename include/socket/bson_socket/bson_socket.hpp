#ifndef MROS_W24_SOLUTION_BSON_SOCKET_HPP
#define MROS_W24_SOLUTION_BSON_SOCKET_HPP



#include <socket/socket.hpp>
#include <socket/utils/peer_closed_exception.hpp>
#include <socket/utils/socket_errno_exception.hpp>
#include <socket/utils/socket_exception.hpp>

#include <vector>
#include <cstdint>
#include <atomic>
#include <queue>
#include <string>
#include <nlohmann/json.hpp>

using Bson = std::vector<std::uint8_t>;
using BsonString = std::basic_string<std::uint8_t>;
using namespace nlohmann;

class BsonSocket : virtual public Socket {
 public:
  /**
   * Default Constructor
   */
  BsonSocket() = default;

  /**
   * Abstract destructor to force subclassing. Closes the socket if it is not already closed.
   */
  ~BsonSocket() override = 0;

  /**
   * Send a bson message in completion by calling send() until all bytes are sent.
   * @param bson The bson message to send.
   * @throws SocketException Throws exception on failure of send() or if this socket is closed, or if message contains a
   * delimiting character.
   * @throws PeerClosedException Throws exception if peer has closed. Users may catch and instantiate a closing
   * sequence.
   */
  void sendMessage(json const& bson);

  /**
   * Receive a Bson message, current impl does not fill the message queue
   * @return The bson message received.
   * @throws SocketException Throws exception on failure of recv() or if this socket it closed.
   * @throws PeerClosedException Throws exception if peer has closed. Users may catch and instantiate a closing
   * sequence.
   */
  json receiveMessage();

  /**
   * Boolean, true if socket file descriptor is open, false otherwise. Defaults to true since derived classes will be
   * open upon successful construction.
   */
  virtual void close();
 protected:
  std::atomic_bool is_open_ = true;
 private:
  //std::queue<BsonString> message_queue_;

  //bool back_is_complete_message_ = false;

  static int constexpr const kReceiveBufferSize_ = 4;

  static std::uint8_t constexpr const kDelimitingCharacter_ = '\n';
};

#endif //MROS_W24_SOLUTION_BSON_SOCKET_HPP

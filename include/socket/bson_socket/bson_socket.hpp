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
// #include <nlohmann/json.hpp>

using Bson = std::vector<std::uint8_t>;
using BsonString = std::basic_string<std::uint8_t>;

class BsonSocket : virtual public Socket {
 public:
  BsonSocket() = default;

  ~BsonSocket() override = 0;

  void sendMessage(Bson const& bson);

  Bson receiveMessage();

  virtual void close();
 protected:
  std::atomic_bool is_open_;
 private:
  //std::queue<BsonString> message_queue_;

  //bool back_is_complete_message_ = false;

  static int constexpr const kReceiveBufferSize_ = 4;

  static std::uint8_t constexpr const kDelimitingCharacter_ = '\n';
};

#endif //MROS_W24_SOLUTION_BSON_SOCKET_HPP

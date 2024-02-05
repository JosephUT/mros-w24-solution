#include "socket/bson_socket/bson_socket.hpp"

#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <sstream>

BsonSocket::~BsonSocket() {
  if (is_open_) {
    int result = ::close(file_descriptor_);
    is_open_.store(false);
  }
}

void BsonSocket::close() {
  if (is_open_) {
    int result = ::close(file_descriptor_);
    is_open_.store(false);
  }
}

void BsonSocket::sendMessage(const json &message) {
  if (!is_open_) throw SocketException("Cannot send on closed socket.");
  pollfd poll_set{};
  poll_set.fd = file_descriptor_;
  poll_set.events = POLLRDHUP;  // Event for peer closing on a stream.
  nfds_t poll_set_count = 1;    // One file descriptor in the poll set.
  int timeout = 1;              // One millisecond timeout.
  int poll_result = poll(&poll_set, poll_set_count, timeout);
  if (poll_result == 1) {
    // This socket registered a read hangup, meaning the peer socket is closed.
    throw PeerClosedException();
  }
  Bson bson = json::to_bson(message);

  // Send size of the bson before sending the actual bson.
  size_t bson_size = bson.size();
  size_t size_to_complete_send = 8;
  ssize_t send_size = 0;
  do {
    send_size =
        send(file_descriptor_, reinterpret_cast<std::uint8_t *>(&bson_size) + send_size, 8 - send_size, MSG_NOSIGNAL);
    size_to_complete_send -= send_size;
    if (send_size == -1) {
      throw SocketErrnoException("Failed to send to peer.");
    }
  } while (size_to_complete_send > 0);

  // Send the actual message.
  send_size = 0;
  size_to_complete_send = bson.size();
  while (size_to_complete_send > 0) {
    send_size = send(file_descriptor_, bson.data() + send_size, bson.size() - send_size, MSG_NOSIGNAL);
    size_to_complete_send -= send_size;
    if (send_size == -1) {
      throw SocketErrnoException("Failed to send to peer.");
    }
  }
}

json BsonSocket::receiveMessage() {
  if (!is_open_) throw SocketException("Cannot receive on closed socket.");
  std::array<std::uint8_t, kReceiveBufferSize_> buffer{};
  BsonString decode_bson;
  ssize_t received_size;

  // Make sure there are enough bytes to decode a size. Front bytes of storage_bson_ are always a size.
  while (storage_bson_.size() < 8) {
    received_size = recv(file_descriptor_, buffer.data(), buffer.size(), 0);
    storage_bson_.append(buffer.data(), received_size);
    if (received_size == 0) {
      throw PeerClosedException();
    } else if (received_size == -1) {
      throw SocketErrnoException("Failed to receive from peer.");
    }
  }

  // Decode the size then erase it from the storage buffer.
  decode_bson.append(storage_bson_.data(), 8);
  size_t bson_size = *reinterpret_cast<size_t *>(decode_bson.data());
  storage_bson_.erase(storage_bson_.begin(), storage_bson_.begin() += 8);

  // Make sure there are enough bytes to decode the bson whose size has just been decoded.
  while (storage_bson_.size() < bson_size) {
    received_size = recv(file_descriptor_, buffer.data(), buffer.size(), 0);
    storage_bson_.append(buffer.data(), received_size);
    if (received_size == 0) {
      throw PeerClosedException();
    } else if (received_size == -1) {
      throw SocketErrnoException("Failed to receive from peer.");
    }
  }

  // Decode the first bson_size bytes and erase them from the storage buffer.
  decode_bson.clear();
  decode_bson.append(storage_bson_.data(), bson_size);
  storage_bson_.erase(storage_bson_.begin(), storage_bson_.begin() += static_cast<int>(bson_size));
  return json::from_bson(decode_bson.begin(), decode_bson.end());
}

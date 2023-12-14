#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <socket/bson_socket/bson_socket.hpp>

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

void BsonSocket::sendMessage(const json &json) {
  if (!is_open_) {
    throw SocketException("Cannot send on closed socket.");
  }
//  if (std::find(bson.begin(), bson.end(), kDelimitingCharacter_) != bson.end()) {
//    throw SocketException("Sending message with delimiting character will cause split receive.");
//  }
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
  // Using 2 size method for Bson
  Bson const bson = json::to_bson(json);
  size_t size_to_complete_send = bson.size();
  ssize_t send_size = 0;
  if (send(file_descriptor_, &size_to_complete_send, sizeof(size_t), 0) == -1) {
    throw SocketErrnoException("Failed to send to peer.");
  }
  do {
    send_size = send(file_descriptor_, reinterpret_cast<const void *>(bson.data() + send_size), bson.size() - send_size,
                     0);  // may need to set the flag to MSG_NOSIGNAL
    size_to_complete_send -= send_size;
    if (send_size == -1) {
      throw SocketErrnoException("Failed to send to peer.");
    }
  } while (size_to_complete_send > 0);
}

json BsonSocket::receiveMessage() {
  if (!is_open_) {
    if (!is_open_) throw SocketException("Cannot receive on closed socket.");
  }
  std::array<std::uint8_t, kReceiveBufferSize_> buffer{};
  // BsonString received_message;
  ssize_t recv_so_far = 0;
  size_t message_size = 0;
  if (recv(file_descriptor_, &message_size, sizeof(size_t), 0) == -1) {
    if (!is_open_) throw SocketErrnoException("Failed to receive from peer.");
  }
  std::vector<std::uint8_t> message(message_size);
  do {
    ssize_t recv_bytes = recv(file_descriptor_, buffer.data(), buffer.size(), 0);
    if (recv_bytes == 0) {
      throw PeerClosedException();
    }
    if (recv_bytes == -1) {
      throw SocketErrnoException("Failed to receive from peer.");
    }
    message.insert(message.begin() + recv_so_far, buffer.begin(), buffer.begin() + recv_bytes);
    recv_so_far += recv_bytes;
  } while (recv_so_far < message_size);
  return json::from_bson(message.begin(), message.end());
}
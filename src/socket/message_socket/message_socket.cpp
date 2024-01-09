#include <socket/message_socket/message_socket.hpp>

MessageSocket::~MessageSocket() {
  // Cannot throw exception in destructor. Assume close is executed without error.
  if (is_open_) {
    int result = ::close(file_descriptor_);
    is_open_ = false;
  }
}

void MessageSocket::sendMessage(std::string message) {
  if (!is_open_) throw SocketException("Cannot send on closed socket.");
  if (message.find(kDelimitingCharacter_) != std::string::npos)
    throw SocketException("Sending message with delimiting character will cause split receive.");

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

  message += kDelimitingCharacter_;
  size_t size_to_complete_send = message.size();
  ssize_t send_size = 0;
  do {
    send_size = send(file_descriptor_, message.data() + send_size,
                     message.size() - send_size, MSG_NOSIGNAL);
    size_to_complete_send -= send_size;
    if (send_size == -1) {
      throw SocketErrnoException("Failed to send to peer.");
    }
  } while (size_to_complete_send > 0);
}

std::string MessageSocket::receiveMessage() {
  if (!is_open_) throw SocketException("Cannot receive on closed socket.");
  std::array<char, kReceiveBufferSize_> receive_buffer{};
  std::string received_string;
  std::string received_message;
  ssize_t received_size;
  int receive_count = 0;

  if (back_is_complete_message_ || message_queue_.size() > 1) {
    // Return the front of the queue as the received message.
    received_message = message_queue_.front();
    message_queue_.pop();
  } else {
    // Call recv() until at least one delimiting character is received or an error is returned.
    do {
      received_size = recv(file_descriptor_, receive_buffer.data(), receive_buffer.size(), 0);
      if (received_size == 0) {
        throw PeerClosedException();
      } else if (received_size == -1) {
        throw SocketErrnoException("Failed to receive from peer.");
      }
      received_string.append(receive_buffer.data(), received_size);
      ++receive_count;
    } while (received_size > 0 && received_string.find(kDelimitingCharacter_) == std::string::npos);
    bool back_is_delimiter = received_string.back() == kDelimitingCharacter_;
    std::istringstream received_stream(received_string);
    std::string substring;

    // Get the received message, prepending previous unfinished message if any.
    std::getline(received_stream, received_message, kDelimitingCharacter_);
    if (!message_queue_.empty()) {
      received_message = message_queue_.front() + received_message;
      message_queue_.pop();
    }

    // Parse any additional substrings separating by delimiter and store them in the message queue.
    while (std::getline(received_stream, substring, kDelimitingCharacter_)) {
      message_queue_.push(substring);
    }
    if (!message_queue_.empty() && back_is_delimiter) {
      back_is_complete_message_ = true;
    } else {
      back_is_complete_message_ = false;
    }
  }  // else
  return received_message;
}

void MessageSocket::close() {
  // Cannot throw exception in destructor. Assume close is executed without error.
  if (is_open_) {
    int result = ::close(file_descriptor_);
    is_open_ = false;
  }
}

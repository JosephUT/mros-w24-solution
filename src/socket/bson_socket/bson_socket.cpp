#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <socket/bson_socket/bson_socket.hpp>
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
    if (!is_open_) { throw SocketException("Cannot send on closed socket."); }
    Bson bson = json::to_bson(message);
    if (std::find(bson.begin(), bson.end(), kDelimitingCharacter_) != bson.end()) {
        throw SocketException("Sending message with delimiting character will cause split receive.");
    }
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

    bson.push_back(kDelimitingCharacter_);
    size_t size_to_complete_send = bson.size();
    ssize_t send_size = 0;
    do {
        send_size = send(file_descriptor_, bson.data() + send_size,
                         bson.size() - send_size,
                         0);  // may need to set the flag to MSG_NOSIGNAL
        size_to_complete_send -= send_size;
        if (send_size == -1) {
            throw SocketErrnoException("Failed to send to peer.");
        }
    } while (size_to_complete_send > 0);
}

json BsonSocket::receiveMessage() {
    if (!is_open_) throw SocketException("Cannot receive on closed socket.");
    std::array<std::uint8_t, kReceiveBufferSize_> buffer{};
    BsonString received_bson_string;
    BsonString received_bson_message;
    ssize_t received_size;
    int receive_count = 0;
    if (back_is_complete_message_ || message_queue_.size() > 1) {
        // Return the front of the queue as the received message.
        received_bson_message = message_queue_.front();
        message_queue_.pop();
    } else {
        // Call recv() until at least one delimiting character is received or an error is returned.
        do {
            received_size = recv(file_descriptor_, buffer.data(), buffer.size(), 0);
            if (received_size == 0) {
                throw PeerClosedException();
            } else if (received_size == -1) {
                throw SocketErrnoException("Failed to receive from peer.");
            }
            received_bson_string.append(buffer.data(), received_size);
            ++receive_count;
        } while (received_size > 0 && received_bson_string.find(kDelimitingCharacter_) == BsonString::npos);
        bool back_is_delimiter = received_bson_string.back() == kDelimitingCharacter_;
        std::basic_istringstream<std::uint8_t> received_stream(received_bson_string);
        BsonString substring;

        // Get the received message, prepending previous unfinished message if any.
        std::getline(received_stream, received_bson_message, kDelimitingCharacter_);
        if (!message_queue_.empty()) {
            received_bson_message = message_queue_.front() + received_bson_message;
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
    }
    return json::from_bson(received_bson_message.begin(), received_bson_message.end());
}


//void BsonSocket::sendMessage(const json &json) {
//    if (!is_open_) {
//        throw SocketException("Cannot send on closed socket.");
//    }
////  if (std::find(bson.begin(), bson.end(), kDelimitingCharacter_) != bson.end()) {
////    throw SocketException("Sending message with delimiting character will cause split receive.");
////  }
//    pollfd poll_set{};
//    poll_set.fd = file_descriptor_;
//    poll_set.events = POLLRDHUP;  // Event for peer closing on a stream.
//    nfds_t poll_set_count = 1;    // One file descriptor in the poll set.
//    int timeout = 1;              // One millisecond timeout.
//    int poll_result = poll(&poll_set, poll_set_count, timeout);
//    if (poll_result == 1) {
//        // This socket registered a read hangup, meaning the peer socket is closed.
//        throw PeerClosedException();
//    }
//    // Using 2 size method for Bson
//    Bson bson = json::to_bson(json);
//    size_t size_to_complete_send = bson.size();
//    ssize_t send_size = 0;
//    if (send(file_descriptor_, &size_to_complete_send, sizeof(size_t), 0) == -1) {
//        throw SocketErrnoException("Failed to send to peer.");
//    }
//    do {
//        send_size = send(file_descriptor_, reinterpret_cast<const void *>(bson.data() + send_size),
//                         bson.size() - send_size,
//                         0);  // may need to set the flag to MSG_NOSIGNAL
//        size_to_complete_send -= send_size;
//        if (send_size == -1) {
//            throw SocketErrnoException("Failed to send to peer.");
//        }
//    } while (size_to_complete_send > 0);
//}
//
//json BsonSocket::receiveMessage() {
//    LogContext context("Bson receiver");
//    if (!is_open_) {
//        if (!is_open_) throw SocketException("Cannot receive on closed socket.");
//    }
//    std::array<std::uint8_t, kReceiveBufferSize_> buffer{};
//    // BsonString received_message;
//    ssize_t recv_so_far = 0;
//    size_t message_size = 0;
//    if (recv(file_descriptor_, &message_size, sizeof(size_t), 0) == -1) {
//        if (!is_open_) throw SocketErrnoException("Failed to receive from peer.");
//    }
//    BsonString message;
//    message.reserve(message_size);
//    do {
//        ssize_t recv_bytes = recv(file_descriptor_, buffer.data(), buffer.size(), 0);
//        if (recv_bytes == 0) {
//            throw PeerClosedException();
//        }
//        if (recv_bytes == -1) {
//            throw SocketErrnoException("Failed to receive from peer.");
//        }
//        message.append(buffer.data(), recv_bytes);
//        recv_so_far += recv_bytes;
//    } while (recv_so_far < message_size);
//    logger_.debug("Attempting to deserialize");
//    json msg = json::from_bson(message.begin(), message.end());
//    message.clear();
//    logger_.debug("Received message: " + msg.dump());
//    return msg;
//}

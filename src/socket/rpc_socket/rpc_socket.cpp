#include <socket/rpc_socket/rpc_socket.hpp>

RPCSocket::RPCSocket() : is_connected_(false) {}

RPCSocket::~RPCSocket() = default;

void RPCSocket::close() {
  // Make the user thread of this socket send a closing message to the receiving thread of peer socket.
  if (is_connected_) {
    sending_lock_.lock();
    is_connected_ = false;
    sendClosingMessage();
    sending_lock_.unlock();

    // Wait for peer socket to return closing message.
    std::unique_lock<std::mutex> unique_closing_lock(closing_lock_);
    closing_condition_variable_.wait(unique_closing_lock, [this] { return this->closing_message_received_; });
  }
}

bool RPCSocket::connected() { return is_connected_; }

void RPCSocket::sendRequest(const CallbackName& callback_name, const std::string& callback_argument) {
  std::string message = callback_name + kParameterDelimiter_ + callback_argument + kParameterDelimiter_;
  sending_lock_.lock();
  try {
    sendMessage(message);
  } catch (PeerClosedException& error) {
  } catch (SocketException& error) {
  }
  sending_lock_.unlock();
}

void RPCSocket::sendRequestAndGetResponse(const CallbackName& callback_name, const std::string& callback_argument,
                                          const CallbackName& response_callback_name) {
  std::string message = callback_name + kParameterDelimiter_ + callback_argument + kParameterDelimiter_ +
                        response_callback_name + kParameterDelimiter_;
  sending_lock_.lock();
  try {
    sendMessage(message);
  } catch (PeerClosedException& error) {
  } catch (SocketException& error) {
  }
  sending_lock_.unlock();
}

void RPCSocket::registerRequestCallback(const CallbackName& callback_name, const RequestCallback& callback) {
  request_callbacks_lock_.lock();
  request_callbacks_[callback_name] = callback;
  request_callbacks_lock_.unlock();
}

void RPCSocket::registerRequestResponseCallback(const CallbackName& callback_name,
                                                const RequestResponseCallback& callback) {
  request_response_callbacks_lock_.lock();
  request_response_callbacks_[callback_name] = callback;
  request_response_callbacks_lock_.unlock();
}

void RPCSocket::registerClosingCallback(const ClosingCallback& callback) {
  closing_callback_lock_.lock();
  closing_callback_set_ = true;
  closing_callback_ = callback;
  closing_callback_lock_.unlock();
}

void RPCSocket::startReceiveCycle() {
  receiving_thread_ = std::thread(&RPCSocket::receiveCycle, this);
  receiving_thread_.detach();
}

void RPCSocket::receiveCycle() {
  std::vector<std::string> message_parameters;
  std::string message_parameter;
  std::string received_message;
  while (true) {
    // Receive message and decide whether it is a request, request and response, or a closing message.
    try {
      received_message = receiveMessage();
    } catch (PeerClosedException& error) {
    } catch (SocketException& error) {
    }
    std::istringstream message_stream(received_message);
    while (std::getline(message_stream, message_parameter, kParameterDelimiter_)) {
      message_parameters.push_back(message_parameter);
    }
    if (message_parameters.size() == 1 || received_message.empty()) {
      // Handle closing message.
      sending_lock_.lock();
      if (is_connected_) {
        is_connected_ = false;
        sendClosingMessage();
      }
      MessageSocket::close();
      sending_lock_.unlock();

      // Execute closing callback if one is registered.
      closing_callback_lock_.lock();
      if (closing_callback_set_) closing_callback_();
      closing_callback_lock_.unlock();

      // Update state of socket and signal any user threads that were waiting on close() for the socket to receive a
      // closing message.
      std::unique_lock<std::mutex> unique_closing_lock(closing_lock_);
      closing_message_received_ = true;
      unique_closing_lock.unlock();
      closing_condition_variable_.notify_all();
      break;
    } else if (message_parameters.size() == 2) {
      // Handle request message.
      processRequest(message_parameters[0], message_parameters[1]);
    } else if (message_parameters.size() == 3) {
      // Handle request response message.
      processRequestResponse(message_parameters[0], message_parameters[1], message_parameters[2]);
    }
    message_parameters.clear();
    received_message.clear();
  }
}

void RPCSocket::processRequest(const CallbackName& callback_name, std::string& callback_argument) {
  request_callbacks_lock_.lock();
  auto request_callback_iterator = request_callbacks_.find(callback_name);
  if (request_callback_iterator != request_callbacks_.end()) {
    request_callback_iterator->second(callback_argument);
  }
  request_callbacks_lock_.unlock();
}

void RPCSocket::processRequestResponse(const CallbackName& callback_name, std::string& callback_argument,
                                       const CallbackName& response_callback_name) {
  std::string callback_result;
  request_response_callbacks_lock_.lock();
  auto request_response_callback_iterator = request_response_callbacks_.find(callback_name);
  if (request_response_callback_iterator != request_response_callbacks_.end()) {
    callback_result = request_response_callback_iterator->second(callback_argument);
    sendRequest(response_callback_name, callback_result);
  }
  request_response_callbacks_lock_.unlock();
}

void RPCSocket::sendClosingMessage() {
  char const* delimiter_pointer = &kParameterDelimiter_;
  std::string message(delimiter_pointer);
  try {
    sendMessage(message);
  } catch (PeerClosedException& error) {
  } catch (SocketException& error) {
  }
}

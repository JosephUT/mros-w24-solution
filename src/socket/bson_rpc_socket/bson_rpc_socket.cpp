#include "socket/bson_rpc_socket/bson_rpc_socket.hpp"

BsonRPCSocket::BsonRPCSocket() : is_connected_(false) {}

BsonRPCSocket::~BsonRPCSocket() = default;

void BsonRPCSocket::close() {
  if (is_connected_) {
    sending_lock_.lock();
    is_connected_.store(false);
    sendClosingMessage();
    sending_lock_.unlock();

    std::unique_lock<std::mutex> unique_closing_lock(closing_lock_);
    closing_condition_variable_.wait(unique_closing_lock, [this]() -> bool { return this->closing_message_received_; });
  }
}

bool BsonRPCSocket::connected() { return is_connected_.load(); }

void BsonRPCSocket::sendRequest(CallbackName const &callback_name, json const &callback_argument) {
  json to_send = {{"callback name", callback_name}, {"message", callback_argument}};
  std::lock_guard<std::mutex> sendingLockGuard(sending_lock_);
  try {
    BsonSocket::sendMessage(to_send);
  } catch (PeerClosedException &error) {
  } catch (SocketException &error) {
  }
}

void BsonRPCSocket::sendRequestAndGetResponse(CallbackName const &callback_name, json const &callback_argument,
                                              CallbackName const &response_callback_name) {
  json to_send = {{"callback name", callback_name},
                  {"response callback name", response_callback_name},
                  {"message", callback_argument}};
  std::lock_guard<std::mutex> sending_lock_guard(sending_lock_);
  try {
    sendMessage(to_send);
    // Received on the recv cycle
  } catch (PeerClosedException &error) {
  } catch (SocketException &error) {
  }
}

void BsonRPCSocket::registerRequestCallback(const CallbackName &callback_name, const RequestCallbackJson &callback) {
  std::lock_guard<std::mutex> request_lock_guard(request_callbacks_lock_);
  request_callbacks_[callback_name] = callback;
  logger_.debug("rcb registered with name: " + callback_name);
}

void BsonRPCSocket::registerRequestResponseCallback(const CallbackName &callback_name,
                                                    const RequestResponseCallbackJson &callback) {
  std::lock_guard<std::mutex> request_response_lock_guard(request_response_callbacks_lock_);
  request_response_callbacks_[callback_name] = callback;
  logger_.debug("rrcb registered with name: " + callback_name);
}

void BsonRPCSocket::registerClosingCallback(const ClosingCallback &callback) {
  std::lock_guard<std::mutex> closing_lock_guard(closing_callback_lock_);
  closing_callback_set_ = true;
  closing_callback_ = callback;
}

void BsonRPCSocket::startReceiveCycle() {
  receiving_thread_ = std::thread(&BsonRPCSocket::receiveCycle, this);
  receiving_thread_.detach();
}

void BsonRPCSocket::receiveCycle() {
  json received_message;
  while (true) {
    try {
      received_message = receiveMessage();
    } catch (PeerClosedException &error) {
    } catch (SocketException &error) {
    }
    logger_.debug("Received message: " + received_message.dump());
    auto closing_message_iter = received_message.find("close");
    auto callback_name_iter = received_message.find("callback name");
    auto request_response_callback_iter = received_message.find("response callback name");
    if (closing_message_iter != received_message.end() || received_message.empty() || received_message.is_discarded()) {
      sending_lock_.lock();
      if (is_connected_) {
        is_connected_.store(false);
        sendClosingMessage();
      }
      BsonSocket::close();
      sending_lock_.unlock();

      // exit closing callback if one is registered;
      closing_callback_lock_.lock();
      if (closing_callback_set_) {
        closing_callback_();
      }
      closing_callback_lock_.unlock();

      std::unique_lock<std::mutex> unique_closing_lock(closing_lock_);
      closing_message_received_ = true;
      unique_closing_lock.unlock();
      closing_condition_variable_.notify_all();
      break;
    } else if (request_response_callback_iter != received_message.end()) {
      processRequestResponse(received_message);
    } else if (callback_name_iter != received_message.end()) {
      processRequest(received_message);
    }
    received_message.clear();
  }
}

void BsonRPCSocket::processRequest(json const &callback_argument) {
  std::lock_guard<std::mutex> lock_guard(request_callbacks_lock_);
  std::string callback_name = callback_argument["callback name"].get<std::string>();
  auto request_callback_iterator = request_callbacks_.find(callback_name);
  if (request_callback_iterator != request_callbacks_.end()) {
    request_callback_iterator->second(callback_argument["message"]);
  } else {
    logger_.debug("callback name " + callback_name + " not found");
  }
}

void BsonRPCSocket::processRequestResponse(nlohmann::json const &callback_argument) {
  LogContext context("processRequestResponse");
  std::lock_guard<std::mutex> lock_guard(request_response_callbacks_lock_);
  std::string const response_callback_name = callback_argument["response callback name"].get<std::string>();
  std::string const callback_name = callback_argument["callback name"].get<std::string>();
  auto request_response_callback_iter = request_response_callbacks_.find(callback_name);
  if (request_response_callback_iter != request_response_callbacks_.end()) {
    json callback_result = request_response_callback_iter->second(callback_argument["message"]);
    sendRequest(response_callback_name, callback_result);
  } else {
    logger_.debug("response callback name " + callback_name + " not found");
  }
}

void BsonRPCSocket::sendClosingMessage() {
  json message = {{"close", closing_callback_set_}};
  try {
    sendMessage(message);
  } catch (PeerClosedException &error) {
  } catch (SocketException &error) {
  }
}

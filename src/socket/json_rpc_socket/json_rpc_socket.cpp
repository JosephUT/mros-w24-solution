#include <socket/json_rpc_socket/json_rpc_socket.hpp>

JsonRPCSocket::JsonRPCSocket() : is_connected_(false) {}

JsonRPCSocket::~JsonRPCSocket() = default;

void JsonRPCSocket::close() {
    if (is_connected_) {
        sending_lock_.lock();
        is_connected_.store(false);
        sendClosingMessage();
        sending_lock_.unlock();

        std::unique_lock<std::mutex> unique_closing_lock(closing_lock_);
        closing_condition_variable_.wait(unique_closing_lock,
                                         [this]() -> bool { return this->closing_message_received_; });
    }
}

bool JsonRPCSocket::connected() { return is_connected_; }

void JsonRPCSocket::sendRequest(const CallbackName &callback_name, nlohmann::json callback_argument) {
    std::lock_guard<std::mutex> sendingLockGuard(sending_lock_);
    try {
        BsonSocket::sendMessage(callback_argument);
    } catch (PeerClosedException &error) {
    } catch (SocketException &error) {
    }
}

void JsonRPCSocket::sendRequestAndGetResponse(const CallbackName &callback_name, nlohmann::json callback_argument,
                                              const CallbackName &response_callback_name) {

    std::lock_guard<std::mutex> sending_lock_guard(sending_lock_);
    try {
        sendMessage(callback_argument);
    } catch (PeerClosedException &error) {
    } catch (SocketException &error) {
    }
}

void JsonRPCSocket::registerRequestCallback(const CallbackName &callback_name, const RequestCallbackJson &callback) {
    std::lock_guard<std::mutex> request_lock_guard(request_callbacks_lock_);
    request_callbacks_[callback_name] = callback;
}

void JsonRPCSocket::registerRequestResponseCallback(const CallbackName &callback_name,
                                                    const RequestResponseCallbackJson &callback) {
    std::lock_guard<std::mutex> request_response_lock_guard(request_response_callbacks_lock_);
    request_response_callbacks_[callback_name] = callback;
}

void JsonRPCSocket::registerClosingCallback(const ClosingCallback &callback) {
    std::lock_guard<std::mutex> closing_lock_guard(closing_callback_lock_);
    closing_callback_set_ = true;
    closing_callback_ = callback;
}

void JsonRPCSocket::startReceiveCycle() {
    receiving_thread_ = std::thread(&JsonRPCSocket::receiveCycle, this);
    receiving_thread_.detach();
}

void JsonRPCSocket::receiveCycle() {
    std::vector<BsonString> message_parameters;
    BsonString message_parameter;
    BsonString received_bson_message;
    while (true) {
        try {
            received_bson_message = receiveMessage();
        } catch (PeerClosedException &error) {
        } catch (SocketException &error) {
        }
        std::basic_istringstream<std::uint8_t> message_stream(received_bson_message);
        while (std::getline(message_stream, message_parameter, kParameterDelimiter_)) {
            message_parameters.push_back(message_parameter);
        }
        if (message_parameters.size() == 1 || received_bson_message.empty()) {
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
        } else if (message_parameters.size() == 2) {
            std::string const callback_name(message_parameters[0].begin(), message_parameters[0].end());
            json callback_argument = json::from_bson(message_parameters[1].begin(), message_parameters[1].end());
            processRequest(callback_name, callback_argument);
        } else if (message_parameters.size() == 3) {
            std::string const callback_name(message_parameters[0].begin(), message_parameters[0].end());
            json callback_argument = json::from_bson(message_parameters[1].begin(), message_parameters[1].end());
            std::string const response_callback_name(message_parameters[2].begin(), message_parameters[2].end());
            processRequestResponse(callback_name, callback_argument, response_callback_name);
        }
        message_parameters.clear();
        received_bson_message.clear();
    }
}

void JsonRPCSocket::processRequest(const CallbackName &callback_name, json &callback_argument) {
    std::lock_guard<std::mutex> lock_guard(request_response_callbacks_lock_);
    auto request_callback_iterator = request_callbacks_.find(callback_name);
    if (request_callback_iterator != request_callbacks_.end()) {
        request_callback_iterator->second(callback_argument);
    }
}

void JsonRPCSocket::processRequestResponse(const CallbackName &callback_name, nlohmann::json &callback_argument,
                                           const CallbackName &response_callback_name) {
    LogContext context("processRequestResponse");
    std::lock_guard<std::mutex> lock_guard(request_response_callbacks_lock_);
    auto request_response_callback_iter = request_response_callbacks_.find(callback_name);
    if (request_response_callback_iter != request_response_callbacks_.end()) {
        logger_.debug("Request Response callback found");
        std::function<json(json &)> jsonFunc = request_response_callback_iter->second;
        logger_.debug("Callback successfully called");
        json callback_result = jsonFunc(callback_argument);
        logger_.debug("Callback result: " + callback_result.dump());
        sendRequest(response_callback_name, callback_result);
    }
}

void JsonRPCSocket::sendClosingMessage() {
    json message = {{"close", closing_callback_set_}};
    try {
        sendMessage(message);
    } catch (PeerClosedException &error) {
    } catch (SocketException &error) {
    }
}

//void JsonRPCSocket::receiveCycle() {
//    LogContext logContext("receiveCycle");
//    json received_message;
//    while (true) {
//        try {
//            received_message = receiveMessage();
//        } catch (PeerClosedException &error) {
//        } catch (SocketException &error) {
//        }
//        auto closing_message_iter = received_message.find("close");
//        auto callback_name_iter = received_message.find("callback name");
//        auto request_response_callback_iter = received_message.find("response callback name");
//        if (closing_message_iter != received_message.end() || received_message.empty()) {
//            sending_lock_.lock();
//            if (is_connected_) {
//                is_connected_.store(false);
//                sendClosingMessage();
//            }
//            BsonSocket::close();
//            sending_lock_.unlock();
//
//            // exit closing callback if one is registered;
//            closing_callback_lock_.lock();
//            if (closing_callback_set_) {
//                closing_callback_();
//            }
//            closing_callback_lock_.unlock();
//
//            std::unique_lock<std::mutex> unique_closing_lock(closing_lock_);
//            closing_message_received_ = true;
//            unique_closing_lock.unlock();
//            closing_condition_variable_.notify_all();
//            break;
//        } else if (request_response_callback_iter != received_message.end()) {
//            LogContext context("request response getter");
//            logger_.debug("Attemtping request response getters");
//            std::string const callback_name = callback_name_iter->get<std::string>();
//            logger_.debug("Got callback name " + callback_name);
//            std::string const response_callback_name = request_response_callback_iter->get<std::string>();
//            logger_.debug("Got response callback name " + response_callback_name);
//            received_message.erase(callback_name_iter);
//            received_message.erase(request_response_callback_iter);
//            processRequestResponse(callback_name, received_message, response_callback_name);
//        } else if (callback_name_iter != received_message.end()) {
//            LogContext context("request getter");
//            logger_.debug("Attempting request getter");
//            std::string const callback_name = callback_name_iter->get<std::string>();
//            logger_.debug("Got callback name " + callback_name);
//            received_message.erase(callback_name_iter);
//            processRequest(callback_name, received_message);
//        } else {
//            throw std::logic_error("No callback name specified");
//        }
//        received_message.clear();
//    }
//}
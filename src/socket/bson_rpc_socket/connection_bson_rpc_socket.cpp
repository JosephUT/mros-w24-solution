#include "socket/bson_rpc_socket/connection_bson_rpc_socket.hpp"

ConnectionBsonRPCSocket::ConnectionBsonRPCSocket(int file_descriptor) : ConnectionSocket(file_descriptor) {
  file_descriptor_ = file_descriptor;
  is_connected_.store(true);
}

void ConnectionBsonRPCSocket::registerConnectingCallback(const RequestCallbackJson &callback) {
  connecting_callback_ = std::optional<RequestCallbackJson>(callback);
}

void ConnectionBsonRPCSocket::startConnection() {
  sendMessage({{"reply", "ack"}});
  json connection_message = receiveMessage();
  if (connecting_callback_) {
    auto connecting_callback = *connecting_callback_;
    connecting_callback(connection_message);
  }
  sendMessage({{"reply", "clr"}});
  startReceiveCycle();
}

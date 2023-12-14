#include <socket/json_rpc_socket/connection_json_rpc_socket.hpp>

JsonConnectionRPCSocket::JsonConnectionRPCSocket(int file_descriptor) : ConnectionSocket(file_descriptor) {
  file_descriptor_ = file_descriptor;
  is_connected_.store(true);
}

void JsonConnectionRPCSocket::startConnection() {
  sendMessage({{"c", "c"}});
  startReceiveCycle();
}
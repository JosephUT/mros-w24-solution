#include <socket/json_rpc_socket/connection_json_rpc_socket.hpp>

ConnectionJsonRPCSocket::ConnectionJsonRPCSocket(int file_descriptor) : ConnectionSocket(file_descriptor) {
  file_descriptor_ = file_descriptor;
  is_connected_.store(true);
}

void ConnectionJsonRPCSocket::startConnection() {
  sendMessage({{"connect", "c"}});
  startReceiveCycle();
}
#include "socket/bson_rpc_socket/connection_bson_rpc_socket.hpp"

ConnectionBsonRPCSocket::ConnectionBsonRPCSocket(int file_descriptor) : ConnectionSocket(file_descriptor) {
  file_descriptor_ = file_descriptor;
  is_connected_.store(true);
}

void ConnectionBsonRPCSocket::startConnection() {
  sendMessage({{"connect", "c"}});
  startReceiveCycle();
}
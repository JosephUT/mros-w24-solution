#include <socket/rpc_socket/connection_rpc_socket.hpp>

ConnectionRPCSocket::ConnectionRPCSocket(int file_descriptor) : ConnectionSocket::ConnectionSocket(file_descriptor) {
  file_descriptor_ = file_descriptor;
  is_connected_ = true;
}

void ConnectionRPCSocket::startConnection() {
  // Send connecting character to Client to unblock client's connect() call.
  sendMessage("c");
  startReceiveCycle();
}

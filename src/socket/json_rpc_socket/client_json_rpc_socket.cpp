#include <socket/json_rpc_socket/client_json_rpc_socket.hpp>
#include <nlohmann/json.hpp>

using namespace nlohmann;

JsonClientRPCSocket::JsonClientRPCSocket(int domain, const std::string &server_address, int port)
    : ClientSocket(domain, server_address, port) {}

void JsonClientRPCSocket::connectToServer(int timeout) {
  ClientSocket::connect();
  waitForConnectionAndReceive(timeout)
}

void JsonClientRPCSocket::waitForConnectionAndReceive(int timeout) {
  pollfd poll_set{};
  poll_set.fd = file_descriptor_;
  poll_set.events = POLLIN;   // Event for data to read, in this case should be connection message from peer.
  nfds_t poll_set_count = 1;  // One file descriptor in the poll set.
  int poll_result = poll(&poll_set, poll_set_count, timeout);
  if (poll_result == 1) {
    // Read out connection message from peer once available, then start receiving thread.
    json connection_message = receiveMessage();
    is_connected_ = true;
    RPCSocket::startReceiveCycle();
  } else {
    throw SocketException("Timed out waiting for connection message");
  }
}
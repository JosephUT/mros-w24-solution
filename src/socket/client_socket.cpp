#include <socket/client_socket.hpp>

ClientSocket::ClientSocket(int domain, const std::string &server_address, int port) {
  // Set up server socket address.
  const char *host_address = server_address.c_str();
  server_address_.sin_family = domain;
  server_address_.sin_port = htons(port);
  if (inet_aton(host_address, &server_address_.sin_addr) == 0) {
    throw SocketException("Failed to convert address.");
  }

  // Set file descriptor accurately and connect to server socket, blocking until server socket calls accept.
  file_descriptor_ = socket(domain, SOCK_STREAM, 0);
}

ClientSocket::~ClientSocket() {}

void ClientSocket::connect() {
  if (::connect(file_descriptor_, reinterpret_cast<struct sockaddr *>(&server_address_), sizeof(server_address_))) {
    throw SocketException("Failed to connect to server.");
  }
}

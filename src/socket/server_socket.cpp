#include <socket/server_socket.hpp>

ServerSocket::ServerSocket(const int domain, const std::string& address, const int port, const int listen_backlog) {
  const char* host_address = address.c_str();

  // Set up server socket address.
  server_address_.sin_family = domain;
  server_address_.sin_port = htons(port);
  if (inet_aton(host_address, &server_address_.sin_addr) == 0) {
    throw SocketErrnoException("Failed to convert address.");
  }

  // Create server socket, set it to be non-blocking, bind it to its address, and listen for connections.
  file_descriptor_ = socket(domain, SOCK_STREAM, 0);
  is_open_ = true;
  int flags = fcntl(file_descriptor_, F_GETFL);
  if (flags == -1) throw SocketException("Failed to get flags.");
  if (fcntl(file_descriptor_, F_SETFL, flags | O_NONBLOCK) == -1) {
    throw SocketErrnoException("Failed to set socket nonblocking.");
  }
  int option = 1;  // Nonzero value to enable boolean option.
  if (setsockopt(file_descriptor_, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
    throw SocketErrnoException("Failed to set socket to reuse address.");
  }
  if (bind(file_descriptor_, reinterpret_cast<struct sockaddr*>(&server_address_), sizeof(server_address_))) {
    throw SocketErrnoException("Failed to bind to port.");
  }
  if (listen(file_descriptor_, listen_backlog)) {
    throw SocketErrnoException("Failed to listen..");
  }

  // If the port is zero the kernel will have assigned the socket a valid address on bind. This recovers the new value.
  if (port == 0) {
    socklen_t length = sizeof(server_address_);
    if (getsockname(file_descriptor_, reinterpret_cast<struct sockaddr*>(&server_address_), &length) < 0) {
      throw SocketErrnoException("Failed to get socket address.");
    }
  }
}

ServerSocket::~ServerSocket() { close(); }

template <class T>
std::shared_ptr<T> ServerSocket::acceptConnection()
  requires std::derived_from<T, ConnectionSocket>
{
  if (!is_open_) throw SocketException("Cannot accept on close socket.");
  sockaddr_in client_address = server_address_;  // Must assign to avoid issues with accept
  socklen_t client_address_size = sizeof(client_address);
  int connection_file_descriptor =
      accept(file_descriptor_, reinterpret_cast<struct sockaddr*>(&client_address), &client_address_size);
  if (connection_file_descriptor == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // Return null option if accept fails because backlog is empty.
      return nullptr;
    }
    // Throw for other errors.
    throw SocketErrnoException("Failed to accept connection.");
  }
  std::array<char, INET_ADDRSTRLEN> client_address_buffer;
  int client_port = ntohs(client_address.sin_port);
  const char* address_ptr =
      inet_ntop(AF_INET, &client_address.sin_addr, client_address_buffer.data(), static_cast<size_t>(INET_ADDRSTRLEN));
  std::string client_address_string(address_ptr);
  last_client_address_port_ = {client_address_string, client_port};

  // Handle appropriate return paths at compile time.
  if constexpr (std::is_same_v<ConnectionSocket, ConnectionBsonSocket>) {
    auto connection = std::make_shared<ConnectionBsonSocket>(connection_file_descriptor);
    return connection;
  } else if constexpr (std::is_same_v<ConnectionSocket, ConnectionBsonRPCSocket>) {
    auto connection = std::make_shared<ConnectionBsonRPCSocket>(connection_file_descriptor);
    return connection;
  } else {
    return std::make_shared<T>(connection_file_descriptor);
  }
}

std::pair<std::string, int> ServerSocket::getAddressPort() {
  char name[INET_ADDRSTRLEN];
  char port_char[10];
  getnameinfo(reinterpret_cast<sockaddr *>(&server_address_), sizeof(server_address_), name,
              sizeof(name), port_char, sizeof(port_char), NI_NUMERICHOST | NI_NUMERICSERV);
  return {std::string(name), stoi(std::string(port_char))};
}

void ServerSocket::close() {
  // Cannot throw exception in destructor. Assume close is executed without error.
  if (is_open_) {
    int result = ::close(file_descriptor_);
    is_open_ = false;
  }
}

// Explicit instantiation to work with BsonSockets.
template std::shared_ptr<ConnectionBsonSocket> ServerSocket::acceptConnection<ConnectionBsonSocket>();

// Explicit instantiation to work with BsonRPCSockets.
template std::shared_ptr<ConnectionBsonRPCSocket> ServerSocket::acceptConnection<ConnectionBsonRPCSocket>();

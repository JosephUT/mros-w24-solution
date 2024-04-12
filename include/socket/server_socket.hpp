#ifndef MROS_W24_SOLUTION_SERVER_SOCKET_HPP
#define MROS_W24_SOLUTION_SERVER_SOCKET_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <cerrno>
#include <concepts>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

#include "socket/bson_rpc_socket/connection_bson_rpc_socket.hpp"
#include "socket/bson_socket/connection_bson_socket.hpp"
#include "socket/connection_socket.hpp"
#include "socket/socket.hpp"
#include "socket/utils/socket_exception.hpp"

/**
 * Class wrapping server sockets. Accepts client connect() calls and returns connected sockets, which should be passed
 * as the template argument.
 */
class ServerSocket : public Socket {
 public:
  /**
   * Initialize a server socket. Calls socket() bind() and listen(), the last of which will begin to store pending
   * connections in the socket's kernel space backlog. Implements a nonblocking socket.
   * @param domain The communication domain code to be used. Currently supports only AF_INET (IPv4).
   * @param address The address to bind to in x.x.x.x format.
   * @param port The port number to bind to.
   * @param listen_backlog The number of pending connections to allow before dropping new connections.
   * @throws SocketException Throws exception on failure of socket(), bind(), or listen().
   */
  ServerSocket(int domain, const std::string &address, int port, int listen_backlog);

  /**
   * Close socket file descriptor on destruction.
   */
  ~ServerSocket() override;

  /**
   * Accept the first connection in the listening backlog and generate a connection socket.
   * Subclasses should template with the associated ConnectionSocket type. These must return an error
   * @return Shared pointer to generated connection socket class instance.
   * @throws SocketException Throws exception on failure to accept.
   */
  template <class T>
  std::shared_ptr<T> acceptConnection() requires std::derived_from<T, ConnectionSocket>;

  /**
   * Get the address and port of the last client to connect.
   * @return A pair containing the client's address and port. Empty if no client has connected.
   */
  std::pair<std::string, int> getLastClientAddressPort() {
    return last_client_address_port_;
  }

  /**
   * Get the address and port of this socket. Useful mainly for recovering kernel-assigned addresses from initializing
   * the port to 0 in the constructor.
   * @return A pair containing this socket's address and port.
   */
  std::pair<std::string, int> getAddressPort();

  /**
   * Close the socket's file descriptor if it is not already closed.
   */
  void close();

 protected:
  /**
   * Struct to store server address.
   */
  sockaddr_in server_address_;

  /**
   * Boolean, true if socket file descriptor is open, false otherwise.
   */
  bool is_open_ = false;

  /**
   * The address and port of the last connection made.
   */
  std::pair<std::string, int> last_client_address_port_;
};

#endif  // MROS_W24_SOLUTION_SERVER_SOCKET_HPP

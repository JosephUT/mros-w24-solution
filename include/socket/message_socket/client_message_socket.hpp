#ifndef MROS_CLIENT_MESSAGE_SOCKET_HPP
#define MROS_CLIENT_MESSAGE_SOCKET_HPP

#include <unistd.h>

#include <socket/client_socket.hpp>
#include <socket/message_socket/message_socket.hpp>

/**
 * Socket class combining string messaging functionality and client style construction.
 */
class ClientMessageSocket : virtual public ClientSocket, virtual public MessageSocket {
 public:
  /**
   * Initialize socket with information of server socket it will connect to.
   * @param domain The communication domain code to be used. Currently supports only AF_INET (IPv4).
   * @param address The address of the server in x.x.x.x format.
   * @param port The port number of the server.
   * @throws SocketException Throw exception if socket() call fails.
   */
  ClientMessageSocket(int domain, const std::string& server_address, int port);

  /**
   * Default destructor to allow for concrete class.
   */
  ~ClientMessageSocket() override = default;
};

#endif  // MROS_CLIENT_MESSAGE_SOCKET_HPP

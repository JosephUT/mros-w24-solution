#ifndef MROS_W24_SOLUTION_CLIENT_SOCKET_HPP
#define MROS_W24_SOLUTION_CLIENT_SOCKET_HPP

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <socket/socket.hpp>
#include <socket/utils/socket_exception.hpp>
#include <string>

/**
 * Abstract class wrapping client sockets.
 */
class ClientSocket : virtual public Socket {
 public:
  /**
   * Initialize socket with information of server socket it will connect to.
   * @param domain The communication domain code to be used. Currently supports only AF_INET (IPv4).
   * @param address The address of the server in x.x.x.x format.
   * @param port The port number of the server.
   * @throws SocketException Throw exception if socket() call fails.
   */
  ClientSocket(int domain, const std::string& server_address, int port);

  /**
   * Pure virtual destructor to force subclassing.
   */
  ~ClientSocket() override = 0;

  /**
   * Attempt to connect to the server specified in the constructor.
   * @throws SocketException Throws exception if connect() call fails.
   */
  virtual void connect();

 protected:
  /**
   * Address of the server the client will connect to on connect().
   */
  sockaddr_in server_address_;
};

#endif  // MROS_W24_SOLUTION_CLIENT_SOCKET_HPP

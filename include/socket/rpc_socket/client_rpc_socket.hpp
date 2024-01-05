#ifndef MROS_CLIENT_RPC_SOCKET_HPP
#define MROS_CLIENT_RPC_SOCKET_HPP

#include <socket/client_socket.hpp>
#include <socket/rpc_socket/rpc_socket.hpp>

/**
 * Class combining string rpc functionality with client style socket initialization.
 */
class ClientRPCSocket : virtual public ClientSocket, virtual public RPCSocket {
 public:
  /**
   * Initialize socket with information of server socket it will connect to.
   * @param domain The communication domain code to be used. Currently supports only AF_INET (IPv4).
   * @param address The address of the server in x.x.x.x format.
   * @param port The port number of the server.
   * @throws SocketException Throw exception if socket() call fails.
   */
  ClientRPCSocket(int domain, const std::string& server_address, int port);

  /**
   * Default destructor to allow for concrete class.
   */
  ~ClientRPCSocket() override = default;

  /**
   * Connects to the server socket. Waits indefinitely for the resulting connection socket to
   * @param timeout Time to wait for a connection message  from the connection socket before throwing an exception in
   * milliseconds. Defaults to an indefinite timeout.
   */
  void connectToServer(int timeout = -1);

 protected:
  /**
   * Move ClientSocket's connect off public interface to wrap starting the receiving thread into the connect logic.
   */
  using ClientSocket::connect;

  /**
   * Poll for an incoming connection message for a specified period. Discard the message if it comes, throw otherwise.
   * @param timeout Time to wait for a connection message from the connection socket before throwing an exception.
   * @throws SocketException Throws exception if timeout expires before a connection message arrives.
   */
  void waitForConnectionAndReceive(int timeout);

  std::thread setup_thread_;
};

#endif  // MROS_CLIENT_RPC_SOCKET_HPP

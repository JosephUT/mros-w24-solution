#ifndef MROS_W24_SOLUTION_CLIENT_BSON_SOCKET_HPP
#define MROS_W24_SOLUTION_CLIENT_BSON_SOCKET_HPP

#include <socket/client_socket.hpp>
#include <socket/bson_socket/bson_socket.hpp>

class BsonClientMessageSocket : virtual public ClientSocket, virtual public BsonSocket {
 public:
  /**
   * Initialize socket with information of server socket it will connect to.
   * @param domain The communication domain code to be used. Currently supports only AF_INET (IPv4).
   * @param address The address of the server in x.x.x.x format.
   * @param port The port number of the server.
   * @throws SocketException Throw exception if socket() call fails.
   */
  BsonClientMessageSocket(int domain, std::string const& server_address, int port);

  /**
   * Default destructor to allow for concrete class.
   */
  ~BsonClientMessageSocket() override = default;
};
#endif //MROS_W24_SOLUTION_CLIENT_BSON_SOCKET_HPP

#ifndef MROS_W24_SOLUTION_CONNECTION_BSON_SOCKET_HPP
#define MROS_W24_SOLUTION_CONNECTION_BSON_SOCKET_HPP

#include <socket/connection_socket.hpp>
#include <socket/bson_socket/bson_socket.hpp>

class BsonConnectionSocket : virtual public ConnectionSocket, virtual public BsonSocket {
 public:
  /**
   * Deleted default constructor to disallow construction by the public.
   */
  BsonConnectionSocket() = delete;

  /**
   * Constructor taking a file descriptor to be called internally to acceptConnection() in ServerSockets.
   * @param file_descriptor The file descriptor to initialize the connection socket with.
   */
  explicit BsonConnectionSocket(int file_descriptor);

  /**
   * Close the socket file descriptor.
   */
  ~BsonConnectionSocket() override = default;
};

#endif //MROS_W24_SOLUTION_CONNECTION_BSON_SOCKET_HPP

#include <socket/connection_socket.hpp>

ConnectionSocket::ConnectionSocket(const int file_descriptor) : Socket(file_descriptor) {}

ConnectionSocket::~ConnectionSocket() = default;

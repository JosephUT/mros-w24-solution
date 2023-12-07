#include <socket/message_socket/connection_message_socket.hpp>

ConnectionMessageSocket::ConnectionMessageSocket(int file_descriptor) : ConnectionSocket(file_descriptor), Socket(file_descriptor) {}

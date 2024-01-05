#include <socket/message_socket/client_message_socket.hpp>

ClientMessageSocket::ClientMessageSocket(int domain, const std::string &server_address, int port)
    : ClientSocket(domain, server_address, port) {}

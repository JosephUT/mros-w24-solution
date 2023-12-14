#include <socket/bson_socket/client_bson_socket.hpp>

BsonClientMessageSocket::BsonClientMessageSocket(int domain, const std::string &server_address, int port)
    : ClientSocket(domain, server_address, port) {}
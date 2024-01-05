#include <socket/socket.hpp>

Socket::Socket() : logger_(Logger::getLogger()) {}

Socket::Socket(int file_descriptor) : file_descriptor_(file_descriptor), logger_(Logger::getLogger()) {}

Socket::~Socket() {}

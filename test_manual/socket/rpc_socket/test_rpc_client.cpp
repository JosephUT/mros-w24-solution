#include <iostream>
#include <memory>
#include <string>

#include "socket/rpc_socket/client_rpc_socket.hpp"

static void printMessage(const std::string& input) {
  std::cout << "printMessage() called with input: " << input << std::endl;
}

static std::string echoAndEmphasize(const std::string& input) {
  std::cout << "echoAndEmphasize() called with input: " << input << std::endl;
  return input + '!';
}

int main(int argc, char** argv) {
  const int kDomain = AF_INET;
  const std::string kServerAddress = "127.0.0.1";
  const int kServerPort = 13347;
  std::string input;
  Logger& logger = Logger::getLogger();
  logger.initialize(argc, argv, "String RPC Client");

  auto client_socket = std::make_shared<ClientRPCSocket>(kDomain, kServerAddress, kServerPort);
  client_socket->registerRequestCallback("printMessage", printMessage);
  client_socket->registerRequestResponseCallback("echoAndEmphasize", echoAndEmphasize);

  // Attempt connection until success in case server has not yet started.
  while (true) {
    try {
      client_socket->connectToServer();
      break;
    } catch (SocketException &error) {}
  }

  // Wait for connection socket to call .close().
  while (client_socket->connected()) {}
  return 0;
}

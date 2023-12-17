#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

#include "socket/json_rpc_socket/client_json_rpc_socket.hpp"

using namespace nlohmann;

void printMessage(const json& input) {
  std::cout << "printMessage() called with input: " << input.dump(0) << std::endl;
}

std::string echo(const json& input) {
  std::cout << "echoAndEmphasize() called with input: " << input.dump(0) << std::endl;
  return input;
}

int main() {
  int const kDomain = AF_INET;
  std::string const kServerAddress = "127.0.0.1";
  int const kServerPort = 13348;

  auto client_sock = std::make_shared<ClientJsonRPCSocket>(kDomain, kServerAddress, kServerPort);
  client_sock->registerRequestCallback("printMessage", &printMessage);
  client_sock->registerRequestResponseCallback("echo", &echo);

  while (true) {
    try {
      client_sock->connectToServer();
      break;
    } catch (SocketException const& e) {}
  }

  while (client_sock->connected()) {}
  return 0;
}
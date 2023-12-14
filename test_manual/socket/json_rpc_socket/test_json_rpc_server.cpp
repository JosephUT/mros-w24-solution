#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>

#include "socket/json_rpc_socket/connection_json_rpc_socket.hpp"
#include "socket/server_socket.hpp"

using namespace nlohmann;

void printMessage(json const& input) { std::cout << "printMessage called with input: " << input.dump(0) << std::endl; }

int main() {
  int const kDomain = AF_INET;
  std::string const kServerAddress = "127.0.0.1";
  int const kServerPort = 13348;
  int const queueSize = 16;

  auto server_socket = std::make_shared<ServerSocket>(kDomain, kServerAddress, kServerPort, queueSize);
  std::optional<std::shared_ptr<ConnectionRPCSocket>> optionalJsonConnectionSocket;
  do {
    optionalJsonConnectionSocket = server_socket->acceptConnection<ConnectionRPCSocket>();
  } while (!optionalJsonConnectionSocket);
  auto connection_socket = *optionalJsonConnectionSocket;

  connection_socket->registerRequestCallback("printMessage", &printMessage);
  connection_socket->startConnection();

  std::string input;
  while (true) {
    std::cout << "enter to send request, quit to exit" << std::endl;
    std::getline(std::cin, input);
    if (input == "quit") break;
    json message = {{input, "Test"}};
    connection_socket->sendRequest("printMessage", message);
    connection_socket->sendRequestAndGetResponse("echo", message, "printMessage");

  }

  return 0;
}
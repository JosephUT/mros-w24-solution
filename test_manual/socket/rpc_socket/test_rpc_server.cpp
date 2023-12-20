#include <iostream>
#include <memory>

#include "socket/rpc_socket/connection_rpc_socket.hpp"
#include "socket/server_socket.hpp"

static void printMessage(const std::string &input) {
    std::cout << "printMessage() called with input: " << input << std::endl;
}

int main(int argc, char **argv) {
    const int kDomain = AF_INET;
    const std::string kServerAddress = "127.0.0.1";
    const int kServerPort = 13347;
    const int kBacklogSize = 16;
    std::string input;
    Logger &logger = Logger::getLogger();
    logger.initialize(argc, argv, "String RPC Client");

    auto server_socket = std::make_shared<ServerSocket>(kDomain, kServerAddress, kServerPort, kBacklogSize);
    std::optional<std::shared_ptr<ConnectionRPCSocket>> optional_connection_socket;
    do {
        // Attempt to accept connections until success in case client has not started.
        optional_connection_socket = server_socket->acceptConnection<ConnectionRPCSocket>();
    } while (!optional_connection_socket);
    auto connection_socket = *optional_connection_socket;

    // Register callback and loop to collect and send requests using user input.
    connection_socket->registerRequestCallback("printMessage", printMessage);
    connection_socket->startConnection();
    while (true) {
        std::cout << "enter to send request, quit to exit" << std::endl;
        std::getline(std::cin, input);
        if (input == "quit") break;
        connection_socket->sendRequest("printMessage", input);
        connection_socket->sendRequestAndGetResponse("echoAndEmphasize", input, "printMessage");
    }
    server_socket->close();
    connection_socket->close();
}

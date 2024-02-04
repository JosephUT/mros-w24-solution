#include <iostream>
#include <logging/logging.hpp>
#include <memory>
#include <nlohmann/json.hpp>
#include <socket/bson_rpc_socket/connection_bson_rpc_socket.hpp>
#include <socket/server_socket.hpp>

using namespace nlohmann;

void printMessage(json const &input) { std::cout << "printMessage called with input: " << input.dump(0) << std::endl; }

int main(int argc, char** argv) {
    try {
        int const kDomain = AF_INET;
        std::string const kServerAddress = "127.0.0.1";
        int const kServerPort = 13348;
        int const queueSize = 16;

        Logger &logger = Logger::getLogger();
        logger.initialize(argc, argv, "JsonRPCServer");
        auto server_socket = std::make_shared<ServerSocket>(kDomain, kServerAddress, kServerPort, queueSize);
        std::shared_ptr<ConnectionBsonRPCSocket> connection_socket;
        do {
            connection_socket = server_socket->acceptConnection<ConnectionBsonRPCSocket>();
        } while (!connection_socket);

        connection_socket->registerRequestCallback("printMessage", &printMessage);
        connection_socket->startConnection();

        std::string input;
        while (true) {
            std::cout << "enter to send request, quit to exit" << std::endl;
            std::getline(std::cin, input);
            if (input == "quit") break;
            json message;
            message["Input"] = input;
            connection_socket->sendRequest("printMessage", message);
            connection_socket->sendRequestAndGetResponse("echo", message, "printMessage");
        }
        server_socket->close();
        connection_socket->close();
    } catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
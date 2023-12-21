#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <logging/logging.hpp>
#include <socket/json_rpc_socket/client_json_rpc_socket.hpp>



using namespace nlohmann;

static void printMessage(const json &input) {
    std::cout << "printMessage() called with input: " << input.dump(0) << std::endl;
}

static json echo(const json &input) {
    std::cout << "echoAndEmphasize() called with input: " << input.dump(0) << std::endl;
    return input;
}

int main(int argc, char** argv) {
    int const kDomain = AF_INET;
    std::string const kServerAddress = "127.0.0.1";
    int const kServerPort = 13348;

    Logger& logger = Logger::getLogger();
    logger.initialize(argc, argv, "JsonRPCclient");

    auto client_sock = std::make_shared<ClientJsonRPCSocket>(kDomain, kServerAddress, kServerPort);
    client_sock->registerRequestCallback("printMessage", printMessage);
    client_sock->registerRequestResponseCallback("echo", echo);

    while (true) {
        try {
            client_sock->connectToServer();
            break;
        } catch (SocketException const &e) {
            logger.debug(e.what());
        }
    }

    while (client_sock->connected()) {}
    std::cout << "Terminating" << std::endl;
    return 0;
}
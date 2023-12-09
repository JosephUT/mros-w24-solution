#include <mros/utils/utils.hpp>

std::string toURI(std::string host, int port) {
    return ("http://" + std::move(host) + ':' + std::to_string(port));
}
#include "mros/utils/utils.hpp"

std::string toURI(const std::string &host, int port) {
  return ("http://" + host + ':' + std::to_string(port));
}

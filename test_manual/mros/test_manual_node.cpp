#include "mros/node.hpp"

#include <thread>
#include <chrono>

using namespace std::chrono_literals;

int main(int argc, char** argv) {
  MROS::init(argc, argv);
  Node test_node("test node");
  while (test_node.connected()) {
    std::this_thread::sleep_for(10ms);
  }
  return 0;
}

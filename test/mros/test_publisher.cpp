#include <iostream>

#include "messages/exampleMessages.hpp"
#include <mros/node.hpp>

int main(int argc, char **argv) {
  MROS::init(argc, argv);

  auto node = std::make_shared<Node>("TestPub");

  auto pub = node->create_publisher<Messages::String>("abc", 10);

  int count = 0;
  while (node->status()) {
    std::cout << "Publishing message" << std::endl;
    Messages::String msg;
    msg.base = "Hello World! " + std::to_string(count++);
    pub->publish(msg);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "Node is shutting down" << std::endl;
  return 0;
}

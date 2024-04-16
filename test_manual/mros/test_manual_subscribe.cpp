#include "messages/example_message.hpp"
#include "mros/node.hpp"

void callback(StringMessage const& msg) { std::cout << "called callback with: " << msg.data << std::endl; }

int main(int argc, char** argv) {
  MROS::init(argc, argv);
  auto test_node = std::make_shared<Node>("test node");
  auto subscriber = test_node->createSubscriber<StringMessage>("test topic", 15, &callback);
  test_node->spin();
  return 0;
}

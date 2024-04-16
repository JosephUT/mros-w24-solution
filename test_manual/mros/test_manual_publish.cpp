#include "messages/example_message.hpp"
#include "mros/node.hpp"

using namespace std::chrono_literals;

int main(int argc, char** argv) {
  StringMessage message;

  MROS::init(argc, argv);
  MROS& mros = MROS::getMROS();
  auto test_node = std::make_shared<Node>("test node");
  auto publisher = test_node->createPublisher<StringMessage>("test topic");

  int count = 0;
  while (mros.active()) {
    message.data = std::to_string(count);
    ++count;

    publisher->publish(message);
    std::this_thread::sleep_for(100ms);
  }
  return 0;
}

#include "messages/exampleMessages.hpp"
#include "mros/node.hpp"

using namespace std::chrono_literals;

void callback(std::string msg) { std::cout << "called callback with: " << msg << std::endl; }

int main(int argc, char** argv) {
  MROS::init(argc, argv);
  auto test_node = std::make_shared<Node>("test node");
  auto subscriber = test_node->createSubscriber<std::string>("test topic", 1, &callback);
  auto publisher_one = test_node->createPublisher<std::string>("test topic");
  {
    auto publisher_two = test_node->createPublisher<std::string>("other topic");
  }
  {
    auto subscriber_two = test_node->createSubscriber<std::string>("additional topic", 1, &callback);
  }
  test_node->spin();
  return 0;
}

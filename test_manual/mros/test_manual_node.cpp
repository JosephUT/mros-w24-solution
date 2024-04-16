#include "messages/example_message.hpp"
#include "mros/node.hpp"

using namespace std::chrono_literals;

void callback(StringMessage const& msg) { std::cout << "called callback with: " << msg.data << std::endl; }

int main(int argc, char** argv) {
  MROS::init(argc, argv);
  auto test_node = std::make_shared<Node>("test node");
  auto subscriber = test_node->createSubscriber<StringMessage>("test topic", 1, &callback);
  auto publisher_one = test_node->createPublisher<StringMessage>("test topic");
  {
    auto publisher_two = test_node->createPublisher<StringMessage>("other topic");
  }
  {
    auto subscriber_two = test_node->createSubscriber<StringMessage>("additional topic", 1, &callback);
  }
  StringMessage message;
  message.data = "sending a test message";
  publisher_one->publish(message);
  test_node->spin();
  return 0;
}

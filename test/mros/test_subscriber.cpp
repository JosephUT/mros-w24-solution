#include <iostream>
#include "messages/exampleMessages.hpp"
#include "mros/node.hpp"


void foo(Messages::String const &msg) {
    std::cout << msg.base << std::endl;
}

int main(int argc, char **argv) {
    MROS::init(argc, argv);

    auto node = std::make_shared<Node>("testSub");

    auto sub = node->create_subscriber<Messages::String>("abc", 10, &foo);
    node->spinOnce();
    node->spin();
    return 0;
}

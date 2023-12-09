#include <iostream>
#include "mros/node.hpp"

int main(int argc, char** argv) {
    MROS::init(argc, argv);
    
    auto node = std::make_shared<Node>("Test1");
    node->spinOnce();
    node->spin();
    return 0;
}

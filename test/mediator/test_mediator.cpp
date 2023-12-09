#include <iostream>
#include "mediator/mediator.hpp"

int main(int argc, char** argv) {
    auto main = std::make_shared<Mediator>(argc, argv);
    main->spin();
    return 0;
}

#include "mediator/mediator.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;



int main(int argc, char **argv) {
  MROS::init(argc, argv);

  auto mediator = std::make_shared<Mediator>();


  while (mediator->status()) {
    std::this_thread::sleep_for(10ms);
  }

  return 0;
}
#include "mediator/mediator.hpp"

int main(int argc, char** argv) {
  MROS::init(argc, argv);
  Mediator mediator("127.0.0.1", 13330);
  return 0;
}
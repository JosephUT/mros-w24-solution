#include "mros/mros.hpp"

#include <exception>
#include <iostream>

MROS *MROS::mros_ptr_ = nullptr;

MROS::MROS(int argc, char **argv) : logger_(Logger::getLogger()), active_(true) {
  logger_.initialize(argc, argv, "MROS");
}

MROS::~MROS() { delete mros_ptr_; }

void MROS::init(int argc, char **argv) {
  if (mros_ptr_) throw std::logic_error("MROS already initialized");
  mros_ptr_ = new MROS(argc, argv);
  std::signal(SIGINT, &MROS::staticHandleSignal);
}

MROS &MROS::getMROS() {
  return *mros_ptr_;
}

bool MROS::active() { return active_; }

void MROS::registerDeactivateRoutine(std::function<void(void)> const &function) {
  deactivate_routines_.push_back(function);
}

void MROS::staticHandleSignal(int signal) {
  if (mros_ptr_) {
    if (signal == SIGINT) {
      mros_ptr_->deactivate();
    }
  }
}

void MROS::deactivate() {
  // Run every registered deactivate routine.
  for (const auto& routine : deactivate_routines_) {
    routine();
  }

  // Set the active flag to false.
  active_ = false;
}

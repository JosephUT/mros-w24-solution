#include "mros/mros.hpp"

#include <exception>

MROS::MROS() : logger_(Logger::getLogger()), check_logger_(false), status_(true) {}

MROS::MROS(int argc, char **argv) : logger_(Logger::getLogger()), check_logger_(true), status_(true) {
  logger_.initialize(argc, argv, "MROS");
  registerHandler();
}

MROS *MROS::mros_ptr_ = nullptr;

void MROS::init(int argc, char **argv) {
  mros_ptr_ = new MROS(argc, argv);
  mros_ptr_->logger_.debug("Logger init called");
  mros_ptr_->registerHandler();
}

MROS &MROS::getMROS() { return *mros_ptr_; }

Logger &MROS::getLogger() {
  if (!check_logger_) {
    throw std::logic_error("MROS never initialized");
  }
  return logger_;
}

bool MROS::status() { return status_; }

void MROS::handleSignal(int signal) { logger_.info("handleSignal: Terminating MROS"); }

void MROS::staticHandleSignal(int signal) {
  if (mros_ptr_) {
    mros_ptr_->logger_.debug("staticHandleSignal(): Signal handler called");
    mros_ptr_->handleSignal(signal);
    if (signal == SIGINT) {
      mros_ptr_->deactivateSignal();
    }
  }
}

void MROS::deactivateSignal() {
  status_.store(false);
  logger_.debug("Deactivating signal");
}

void MROS::registerHandler() {
  mros_ptr_ = this;
  std::signal(SIGINT, &MROS::staticHandleSignal);
}

MROS::~MROS() { delete mros_ptr_; }

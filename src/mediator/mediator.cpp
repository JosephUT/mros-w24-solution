#include "mediator/mediator.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <thread>
#include <algorithm>
#include <errno.h>

#include "logging/logging.hpp"

//---------------------------------------------Mediator Signal Handler--------------------------------------

MediatorSignalHandler::MediatorSignalHandler() : logger_(Logger::getLogger()) {}

MediatorSignalHandler::MediatorSignalHandler(int argc, char **argv) : logger_(Logger::getLogger()) {
    logger_.initialize(argc, argv, "MediatorSignalHandler");
}

MediatorSignalHandler *MediatorSignalHandler::instancePtr = nullptr;

void MediatorSignalHandler::handleSignal(int signal) {
    LogContext context("MediatorSignalHandler");
    logger_.info("handleSignal(): Terminating SignalHandler core with signal " + std::string(strsignal(signal)));
}

void MediatorSignalHandler::staticHandleSignal(int signal) {
    if (instancePtr) {
        instancePtr->handleSignal(signal);
        if (signal == SIGINT) {
            instancePtr->deactivateSignalHandler();
        }
    }
}

void MediatorSignalHandler::registerHandler() {
    instancePtr = this;
    std::signal(SIGINT, &MediatorSignalHandler::staticHandleSignal);
    status.store(true);
}

bool MediatorSignalHandler::getStatus() {
    return this->status;
}

void MediatorSignalHandler::deactivateSignalHandler() {
    status.store(false);
}

//---------------------------------------------Mediator--------------------------------------

Mediator::Mediator(int argc, char **argv) : Mediator(argc, argv, "127.0.0.1", 13330) {}

Mediator::Mediator(int argc, char **argv, std::string address, int port) : address_(std::move(address)), port_(port),
                                                                           logger_(Logger::getLogger()) {
    logger_.initialize(argc, argv, "Mediator");
    handler.registerHandler();

    // createMainSocket();

    logger_.debug("Mediator initialized");
}

void Mediator::spin() {
    LogContext context("Mediator::spin()");
    logger_.info("Spinning SignalHandler Core");

    while(status()) {
        std::this_thread::sleep_for(100ms);
    }

    logger_.info("Exiting spin()");
    // startListening();
}

Mediator::~Mediator() {
    LogContext context("~Mediator");
    logger_.debug("Cleaning up");

    // close(server_fd_);
    logger_.debug("Mediator destructor complete");
}

#include <iostream>

int main(int argc, char** argv) {
    auto main = std::make_shared<Mediator>(argc, argv);
    main->spin();
    return 0;
}

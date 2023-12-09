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
#include "mros/utils/utils.hpp"

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
    LogContext context("Mediator::Mediator");
    logger_.debug("Initializing Mediator");
    handler.registerHandler();

    // createMainSocket();

    RPCListenerThread_ = std::thread(&Mediator::RPCListenerThread, this);

    logger_.debug("Mediator initialized");
}

void Mediator::spin() {
    LogContext context("Mediator::spin()");
    logger_.debug("Spinning SignalHandler Core");

    while(status()) {
        std::this_thread::sleep_for(100ms);
    }

    logger_.debug("Exiting spin()");
    // startListening();
}

Mediator::~Mediator() {
    LogContext context("Mediator::~Mediator");
    logger_.debug("Cleaning up");

    RPCListenerThread_.join();
    logger_.debug("After RPCListenerThread join");

    // close(server_fd_);
    logger_.debug("Mediator destructor complete");
}

//TODO This is a thread that listens for and responds to incoming RPC calls.
void Mediator::RPCListenerThread() {
    LogContext context("Mediator::RPCListenerThread");
    logger_.debug("Starting RPCListenerThread");
    while(status()) {
        std::this_thread::sleep_for(100ms);
    }
    logger_.debug("Exiting RPCListenerThread");
}

//TODO not sure if these are needed in this way
void Mediator::addSubscriber(std::string const &topic_name, std::string const &host, int const port) {
    LogContext context("Mediator::addSubscriber");
    logger_.debug("Adding subscriber to topic " + topic_name + " at " + host + ":" + std::to_string(port));

    subMutex_.lock();
    // subscriberTable_[toURI(host, port)].emplace_back(topic_name);
    subMutex_.unlock();

    pubMutex_.lock();
    //Get publishers
    pubMutex_.unlock();
    //TODO send to new subscriber the list of publishers
}

void Mediator::addPublisher(std::string const &topic_name, std::string const &host, int const port) {
    LogContext context("Mediator::addPublisher");
    logger_.debug("Adding publisher to topic " + topic_name + " at " + host + ":" + std::to_string(port));

    pubMutex_.lock();
    // publisherTable_[toURI(host, port)].emplace_back(topic_name);
    // std::vector<Topic> publishers = publisherTable_[topic_name];
    pubMutex_.unlock();

    subMutex_.lock();
    // std::vector<Topic> subscribers = subscriberTable_[topic_name];
    subMutex_.unlock();

    // for (auto &subscriber : subscribers) {
        //TODO send to subscriber the updated list of publishers
    // }
}

void Mediator::removeSubscriber(std::string const &topic_name, std::string const &host, int const port) {
    LogContext context("Mediator::removeSubscriber");
    logger_.debug("Removing subscriber from topic " + topic_name + " at " + host + ":" + std::to_string(port));

    subMutex_.lock();
    // subscriberTable_[topic_name].erase(std::remove(subscriberTable_[topic_name].begin(), subscriberTable_[topic_name].end(), toURI(host, port)), subscriberTable_[topic_name].end());
    subMutex_.unlock();

    // No sending here
}

void Mediator::removePublisher(std::string const &topic_name, std::string const &host, int const port) {
    LogContext context("Mediator::removePublisher");
    logger_.debug("Removing publisher from topic " + topic_name + " at " + host + ":" + std::to_string(port));

    pubMutex_.lock();
    // publisherTable_[topic_name].erase(std::remove(publisherTable_[topic_name].begin(), publisherTable_[topic_name].end(), toURI(host, port)), publisherTable_[topic_name].end());
    // std::vector<Topic> publishers = publisherTable_[topic_name];
    pubMutex_.unlock();

    subMutex_.lock();
    // std::vector<Topic> subscribers = subscriberTable_[topic_name];
    subMutex_.unlock();

    // for (auto &subscriber : subscribers) {
        //TODO send to subscriber the updated list of publishers
    // }
}

void Mediator::addNode(std::string const &node_name, std::string const &host, int const port) {
    LogContext context("Mediator::addNode");
    logger_.debug("Adding node " + node_name + " at " + host + ":" + std::to_string(port));

    std::lock_guard<std::mutex> lock(nodeMutex_);
    nodeTable_[toURI(host, port)] = node_name;

    //TODO any calls to be made?
}

void Mediator::removeNode(std::string const &node_name, std::string const &host, int const port) {
    LogContext context("Mediator::removeNode");
    logger_.debug("Removing node " + node_name + " at " + host + ":" + std::to_string(port));

    std::lock_guard<std::mutex> lock(nodeMutex_);
    nodeTable_.erase(toURI(host, port));

    //TODO any calls to be made?
}


#ifndef MROS_W24_SOLUTION_PUBLISHER_HPP
#define MROS_W24_SOLUTION_PUBLISHER_HPP

#include <iostream>
#include <memory>
#include <cstdint>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <atomic>
#include <unordered_set>
#include <unistd.h>

#include "logging/logging.hpp"
#include "mros/mros.hpp"
// #include "jsonRPC.hpp"

class PublisherBase {
public:
    friend class Node;

    virtual ~PublisherBase() = default;
};

class Node;

template<typename MessageT>
class Publisher : public std::enable_shared_from_this<Publisher<MessageT>>, public PublisherBase {
public:
    Publisher() = delete;

    ~Publisher() override;

    Publisher(std::weak_ptr<Node> node, std::string topic_name, std::uint32_t queue_size);

    void publish(MessageT const &msg);

    std::string getTopicName() const;

private:
    bool status();

    std::string topic_name_;
    std::uint32_t queue_size_;
    
    std::weak_ptr<Node> node_;
    
    Logger &logger_;
    MROS &core_;


};

template<typename MessageT>
Publisher<MessageT>::Publisher(std::weak_ptr<Node> node, std::string topic_name,
                               std::uint32_t queue_size) : node_(std::move(node)),
                                                           topic_name_(std::move(topic_name)),
                                                           queue_size_(queue_size),
                                                           logger_(Logger::getLogger()),
                                                           core_(MROS::getMROS()){
    LogContext context("Publisher::Publisher");
    logger_.debug("Initializing Publisher");
    core_.registerHandler();

    logger_.debug("Publisher constructor complete");
}

template<typename MessageT>
Publisher<MessageT>::~Publisher() {
    LogContext context("Publisher::~Publisher");
    logger_.debug("Cleaning up");

    logger_.debug("Publisher destructor complete");
}

template<typename MessageT>
void Publisher<MessageT>::publish(const MessageT &msg) {
    LogContext context("Publisher::publish()");

    logger_.debug("Exiting publish()");
}

template<typename MessageT>
std::string Publisher<MessageT>::getTopicName() const {
    return topic_name_;
}

template<typename MessageT>
bool Publisher<MessageT>::status() {
    return core_.status();
}

#endif //MROS_W24_SOLUTION_PUBLISHER_HPP

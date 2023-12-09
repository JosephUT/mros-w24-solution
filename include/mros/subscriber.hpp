#ifndef MROS_W24_SOLUTION_SUBSCRIBER_HPP
#define MROS_W24_SOLUTION_SUBSCRIBER_HPP

#include <memory>
#include <cstdint>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <unordered_set>
#include <thread>

#include "messages/exampleMessages.hpp"
#include "logging/logging.hpp"
#include "mros/mros.hpp"
// #include "jsonRPC.hpp"

using namespace std::chrono_literals;

class SubscriberBase {
public:
    friend class Node;

    virtual ~SubscriberBase() = default;

private:
    virtual void spin() = 0;

    virtual void spinOnce() = 0;
};

class Node;

template<typename MessageT>
class Subscriber : public std::enable_shared_from_this<Subscriber<MessageT>>, public SubscriberBase {
public:
    Subscriber() = delete;

    ~Subscriber() override;

    Subscriber(std::weak_ptr<Node> node, std::string topic_name, std::uint32_t queue_size, std::function<void(MessageT)> callback);

    std::string getTopicName() const;

private:
    void spin() override;

    void spinOnce() override;

    bool status();

    std::string topic_name_;
    std::uint32_t queue_size_;

    std::function<void(MessageT)> callbackFunc;
    
    std::weak_ptr<Node> node_;
    
    Logger &logger_;
    MROS &core_;
};


template<typename MessageT>
Subscriber<MessageT>::Subscriber(std::weak_ptr<Node> node, std::string topic_name, std::uint32_t queue_size,
                                     std::function<void(MessageT)> callback) : node_(std::move(node)),
                                                                               topic_name_(std::move(
                                                                                       topic_name)),
                                                                               queue_size_(queue_size),
                                                                               callbackFunc(callback),
                                                                               logger_(Logger::getLogger()),
                                                                               core_(MROS::getMROS()) {
    LogContext context("Subscriber::Subscriber");
    logger_.debug("Initializing Subscriber");
    core_.registerHandler();

    logger_.debug("Subscriber constructor complete");
}

template<typename MessageT>
Subscriber<MessageT>::~Subscriber() {
    LogContext context("Subscriber::~Subscriber");
    logger_.debug("Cleaning up");

    logger_.debug("Subscriber destructor complete");
}

template<typename MessageT>
std::string Subscriber<MessageT>::getTopicName() const {
    return topic_name_;
}

template<typename MessageT>
void Subscriber<MessageT>::spin() {
    LogContext context("Subscriber::spin()");
    logger_.debug("spinning");

    while(status()) {
        std::this_thread::sleep_for(100ms);
    }

    logger_.debug("Exiting spin");
}

template<typename MessageT>
void Subscriber<MessageT>::spinOnce() {
    LogContext context("Subscriber::spinOnce()");
    logger_.debug("Spinning once");

    std::this_thread::sleep_for(100ms);

    logger_.debug("Exiting spinOnce");
}

template<typename MessageT>
bool Subscriber<MessageT>::status() {
    return core_.status();
}


#endif //MROS_W24_SOLUTION_SUBSCRIBER_HPP

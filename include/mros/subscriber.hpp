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
#include <unordered_map>
#include <thread>
#include <condition_variable>

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

    void receiverThread(int socket_fd);

    std::string topic_name_;
    std::uint32_t queue_size_;

    std::function<void(MessageT)> callbackFunc;
    
    std::weak_ptr<Node> node_;

    std::condition_variable spin_cv; //Broadcasted when spin() or spinOnce() is called. Blocks all receivers.

    std::unordered_map<int, std::thread> publisherListenerThreads_; // socket fd -> thread. Makes sure we have one thread per socket exactly
    std::mutex publisherListenerThreadsMutex_;
    int numThreadsWaiting_ = 0;
    bool listenerThreadsCondition = false;

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

    // Just for debug: make three fake receiver threads, fd 1,2,3
    // for(int i = 1; i <= 3; i++) {
    //     publisherListenerThreads_[i] = std::thread(&Subscriber::receiverThread, this, i);
    // }

    logger_.debug("Subscriber constructor complete");
}

template<typename MessageT>
Subscriber<MessageT>::~Subscriber() {
    LogContext context("Subscriber::~Subscriber");
    logger_.debug("Cleaning up");

    //TODO not sure if threads would be stuck at wait() - better to notify to make sure they get to the bottom.
    {
        std::lock_guard<std::mutex> lock(publisherListenerThreadsMutex_);
        listenerThreadsCondition = true;
    }
    spin_cv.notify_all();

    for(auto &thread : publisherListenerThreads_) {
        if(thread.second.joinable()){
            thread.second.join();
        }
    }

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
        spin_cv.notify_all();
        std::this_thread::sleep_for(100ms);
    }

    logger_.debug("Exiting spin");
}

template<typename MessageT>
void Subscriber<MessageT>::spinOnce() {
    LogContext context("Subscriber::spinOnce()");
    logger_.debug("Spinning once");

    {
        std::lock_guard<std::mutex> lock(publisherListenerThreadsMutex_);
        listenerThreadsCondition = true;
    }
    spin_cv.notify_all();

    //TODO wait for all threads to be waiting again
    std::unique_lock<std::mutex> lock(publisherListenerThreadsMutex_);
    while(numThreadsWaiting_ != publisherListenerThreads_.size()) {
        spin_cv.wait(lock, [this]{return numThreadsWaiting_ == publisherListenerThreads_.size();});
    }

    // std::this_thread::sleep_for(100ms);

    logger_.debug("Exiting spinOnce");
}

template<typename MessageT>
bool Subscriber<MessageT>::status() {
    return core_.status();
}

template<typename MessageT>
void Subscriber<MessageT>::receiverThread(int socket_fd) {
    LogContext context("Subscriber::receiverThread()");
    logger_.debug("Spinning receiverThread");

    while(status()) {
        {
            std::unique_lock<std::mutex> lock(publisherListenerThreadsMutex_);
            numThreadsWaiting_++;
            spin_cv.wait(lock, [this]{return listenerThreadsCondition;});
            numThreadsWaiting_--;
        }
        logger_.debug("CV received");
        
        std::this_thread::sleep_for(100ms);

        //TODO receive message from socket bla bla
    }

    logger_.debug("Exiting receiverThread");
}

#endif //MROS_W24_SOLUTION_SUBSCRIBER_HPP

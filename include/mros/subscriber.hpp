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

class Node;

/*
 * Non-templated base class for ownership
 */
class SubscriberBase {
public:
    friend class Node;

    virtual ~SubscriberBase() = default;

private:
    virtual void spin() = 0;

    virtual void spinOnce() = 0;
};

class Node;
/**
 * Subscriber class to send messages
 * @tparam MessageT Type of message to be sent. Requires conversion to and from json
 */
template<typename MessageT>
class Subscriber : public std::enable_shared_from_this<Subscriber<MessageT>>, public SubscriberBase {
public:
    friend class Node;

    /**
     * Deleted base ctor for Subscriber
     */
    Subscriber() = delete;

    /**
     * Ctor for subscriber. Please use Node::create_subscriber
     * @param node Non-owning node pointer
     * @param topic_name String name of topic
     * @param queue_size The number of messages allowed on the queue before refusal
     * @param callback The callback function to be called
     */
    Subscriber(std::weak_ptr<Node> node, std::string topic_name, std::uint32_t queue_size, std::function<void(MessageT)> callback);

    /**
     * Overriden destructor for subscriber
     * TODO: outline functionality
     */
    ~Subscriber() override;

    /**
     * Trivial accessor for topic name
     * @return topic name
     */
    std::string getTopicName() const;

private:

    /**
     * Handles continous incoming messages to be called
     */
    void spin() override;

    /**
     * Handles message once
     */
    void spinOnce() override;

    /**
     * Access status of MROS
     * @return MROS status
     */
    bool status();

    /**
     * Creates thread to receive incoming messages on one file descriptor
     * @param socket_fd File descriptor
     */
    void receiverThread(int socket_fd);

    /**
     * string name of topic
     */
    std::string topic_name_;

    /**
     * number of messages to be allowed on the queue
     */
    std::uint32_t queue_size_;

    /**
     * Callback to be called with. Takes in Message of type MessageT
     */
    std::function<void(MessageT)> callbackFunc;

    /**
     * Non-owning pointer to node used to construct object
     */
    std::weak_ptr<Node> node_;

    /**
     * Conidition variable that locks receiving messages until new publisher is added to list and connection is created
     */
    std::condition_variable spin_cv; //Broadcasted when spin() or spinOnce() is called. Blocks all receivers.

    /**
     * Lookup socket file descriptor -> thread. One to one
     */
    std::unordered_map<int, std::thread> publisherListenerThreads_;

    /**
     * Mutex for publisherListenerThreads_ to prevent race conditions
     */
    std::mutex publisherListenerThreadsMutex_;

    /**
     * Number of threads waiting
     * TODO: More information
     */
    int numThreadsWaiting_ = 0;

    /**
     * TODO: Unsure
     */
    bool listenerThreadsCondition = false;

    /**
     * Reference to logger
     */
    Logger &logger_;

    /**
     * Reference to MROS
     */
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

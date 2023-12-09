
#include "mros/node.hpp"

Node::Node(const std::string &node_name) : logger_(Logger::getLogger()), core_(MROS::getMROS()) {
    core_.registerHandler();
}

Node::~Node() {
    LogContext context("~Node");
    logger_.debug("Cleaning up");

    logger_.info("Node destructor complete");
}

// template<typename MessageT, typename CallbackT, typename SubscriptionT>
// std::shared_ptr<SubscriptionT>
// Node::create_subscription(std::string topic_name, std::uint32_t queue_size, CallbackT &&callback) {
//     std::function<void(MessageT)> callbackFunc(callback);
//     auto temp = std::make_shared<SubscriptionT>(shared_from_this(), std::move(topic_name),
//                                                 MessageT::messageName(),
//                                                 queue_size, std::move(callbackFunc));
//     // subs_.push_back(temp);
//     return temp;
// }

// template<typename MessageT, typename PublisherT>
// std::shared_ptr<PublisherT> Node::create_publisher(std::string topic_name, uint32_t queue_size) {
//     auto temp = std::make_shared<PublisherT>(shared_from_this(), std::move(topic_name),
//                                              queue_size);
//     // pubs_.push_back(temp);
//     return temp;
// }


bool Node::status() const {
    return core_.status();
}

void Node::spin() {
    LogContext context("Node::spin()");
    logger_.info("spinning");

    while(status()) {
        std::this_thread::sleep_for(100ms);
    }

    logger_.info("Exiting spin");
}

void Node::spinOnce() {
    LogContext context("Node::spinOnce()");
    logger_.info("Spinning once");

    std::this_thread::sleep_for(100ms);

    logger_.info("Exiting spinOnce");
}

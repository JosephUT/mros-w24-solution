#ifndef MROS_W24_SOLUTION_NODE_HPP
#define MROS_W24_SOLUTION_NODE_HPP

#include <iostream>
#include "messages/exampleMessages.hpp"
#include <memory>
#include <functional>
#include <vector>
#include <thread>
#include <chrono>

// #include "subscriber.hpp"
#include "mros/publisher.hpp"
#include "logging/logging.hpp"
#include "mros/mros.hpp"

using namespace std::chrono_literals;

class Node : public std::enable_shared_from_this<Node> {
public:

    explicit Node(std::string const &node_name);

    ~Node();

    // template<typename MessageT, typename CallbackT = void (*)(MessageT), typename SubscriptionT = Subscription<MessageT>>
    // std::shared_ptr<SubscriptionT>
    // create_subscription(std::string topic_name, std::uint32_t queue_size, CallbackT &&callback);

    template<typename MessageT, typename PublisherT = Publisher<MessageT>>
    std::shared_ptr<PublisherT> create_publisher(std::string topic_name, uint32_t queue_size);

    void spin();

    void spinOnce();

    bool status() const;

private:
    std::string node_name_;

    Logger &logger_;
    MROS &core_;

    // std::vector<std::weak_ptr<SubscriptionBase>> subs_;
    // std::vector<std::weak_ptr<PublisherBase>> pubs_;
    // std::vector<std::thread> threadPool_;
};

template<typename MessageT, typename PublisherT>
std::shared_ptr<PublisherT> Node::create_publisher(std::string topic_name, uint32_t queue_size) {
    auto temp = std::make_shared<PublisherT>(shared_from_this(), std::move(topic_name), queue_size);
    // pubs_.push_back(temp);
    return temp;
}

#endif //MROS_W24_SOLUTION_NODE_HPP

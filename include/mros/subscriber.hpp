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
#include "utils.hpp"

#include "messages/exampleMessages.hpp"
#include "logging/logging.hpp"
#include "mros/mros.hpp"
// #include "jsonRPC.hpp"

class SubscriberBase {
public:
    friend class Node;

    virtual ~SubscriberBase() = default;

private:
    virtual void spin() = 0;
};

class Node;

template<typename MessageT>
class Subscriber : public std::enable_shared_from_this<Subscriber<MessageT>>, public SubscriberBase {
public:
    Subscriber() = delete;

    ~Subscriber() override;

    Subscriber(std::weak_ptr<Node> node, std::string topic_name, std::string topic_type,
                 std::uint32_t queue_size, std::function<void(MessageT)> callback);

    std::string getTopicName() const;

private:
    void spin() override;
};

#endif //MROS_W24_SOLUTION_SUBSCRIBER_HPP

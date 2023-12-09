#include "mros/publisher.hpp"

template<typename MessageT>
std::string Publisher<MessageT>::getTopicName() const {
    return topic_name_;
}

template<typename MessageT>
bool Publisher<MessageT>::status() {
    return core_.status();
}


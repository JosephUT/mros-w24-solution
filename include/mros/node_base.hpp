#pragma once

#include <string>

/**
 * Interface class for Node to provide dependency inversion with Publisher and Subscriber.
 */
class NodeBase {
  template <typename MessageT>
  friend class Publisher;

  template <typename MessageT>
  friend class Subscriber;
 protected:
  NodeBase() = default;

  virtual ~NodeBase() = default;

  virtual void removeSubscriberByTopic(std::string topic_name) = 0;

  virtual void removePublisherByTopic(std::string topic_name) = 0;
};

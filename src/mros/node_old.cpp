#include "mros/node.hpp"

Node::Node(const std::string &node_name) : logger_(Logger::getLogger()), core_(MROS::getMROS()) {
  LogContext context("Node::Node");
  logger_.debug("Initializing Node");
  core_.registerHandler();

  // set up data

  // set up client rpc socket with mediator server address

  // get host:port of client rpc socket and resolve to node URI

  // make connecting json message with node_uri and node_name

  // connect to the mediator server, receiving cycle starts after this call

  // start status checking thread which will handle disconnecting everything and killing all finishing all the internal threads of the node

  logger_.debug("Node constructor complete");
}

Node::~Node() {
  LogContext context("~Node");
  logger_.debug("Cleaning up");

  // for(auto &pub : pubs_) {
  //     if(auto ptr = pub.lock()) {
  //         ptr->~PublisherBase();
  //     }
  // }
  // for (auto &sub : subs_) {
  //     if (auto ptr = sub.lock()) {
  //         ptr->~SubscriberBase();
  //     }
  // }
  // logger_.debug("Pubs and Subs cleaned up");

  logger_.debug("Node destructor complete");
}

bool Node::status() const {
  return core_.status();
}

void Node::spin() {
  LogContext context("Node::spin()");
  logger_.debug("spinning");

  std::vector<std::thread> threadPool_;

  for (auto &sub : subs_) {
    if (auto ptr = sub.lock()) {
      threadPool_.emplace_back([ptr]() {ptr->spin();});
    }
  }

  while(status());

  logger_.debug("Joining spin threads");

  for(auto &thread : threadPool_) {
    thread.join();
  }

  logger_.debug("Exiting spin");
}

void Node::spinOnce() {
  LogContext context("Node::spinOnce()");
  logger_.debug("Spinning once");

  std::vector<std::thread> threadPool_;

  for(auto &sub : subs_) {
    if(auto ptr = sub.lock()) {
      threadPool_.emplace_back([ptr]() {ptr->spinOnce();});
    }
  }
  logger_.debug("Joining spinOnce threads");

  for(auto &thread : threadPool_) {
    thread.join();
  }
  // std::this_thread::sleep_for(100ms);

  logger_.debug("Exiting spinOnce");
}

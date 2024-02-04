#include "mediator/mediator.hpp"
#include <sys/socket.h>
#include <string>
#include <thread>
#include <algorithm>

#include <logging/logging.hpp>
#include <mros/utils/utils.hpp>


//---------------------------------------------Mediator--------------------------------------

Mediator::Mediator() : Mediator("127.0.0.1", 13330) {}

Mediator::Mediator(std::string address, int port) : address_(std::move(address)), port_(port),
                                                                           mros_(MROS::getMROS()),
                                                                           logger_(Logger::getLogger()) {
    LogContext context("Mediator::Mediator");

    // createMainSocket();
    json_rpc_server_ = std::make_unique<ServerSocket>(AF_INET ,address_, port_, 100);

    RPCListenerThread_ = std::thread(&Mediator::RPCListenerThread, this);

    logger_.debug("Mediator initialized");
}

void Mediator::spin() {
    LogContext context("Mediator::spin()");
    logger_.debug("Spinning SignalHandler Core");

    while(status()) {
        std::this_thread::sleep_for(100ms);
    }

    logger_.debug("Exiting spin()");
    // startListening();
}

Mediator::~Mediator() {
    LogContext context("Mediator::~Mediator");
    logger_.debug("Cleaning up");

    RPCListenerThread_.join();
    logger_.debug("After RPCListenerThread join");

    // close(server_fd_);
    logger_.debug("Mediator destructor complete");
}

//TODO This is a thread that listens for and responds to incoming RPC calls.
void Mediator::RPCListenerThread() {
    LogContext context("Mediator::RPCListenerThread");
    logger_.debug("Starting RPCListenerThread");
    while(status()) {
        std::shared_ptr<ConnectionBsonRPCSocket> connection_socket;
        do {
          connection_socket = json_rpc_server_->acceptConnection<ConnectionBsonRPCSocket>();
        } while (status() && !connection_socket);

        if (!status()) {
          continue;
        }
        connection_socket->registerRequestResponseCallback("subscribe", [this](Json const& json1) -> Json {
          return jsonSubscribeCallback(json1);
        });

        connection_socket->registerRequestCallback("unsubscribe", [this](Json const& json1) -> Json {
          return jsonUnsubscribeCallback(json1);
        });

        connection_socket->registerRequestCallback("publish", [this](Json const& json1) -> Json {
          return jsonPublishCallback(json1);
        });

        connection_socket->registerRequestCallback("unpublish", [this](Json const& json1) -> Json {
          return jsonUnpublishCallback(json1);
        });

        connection_socket->startConnection();

        connection_list_.insert(connection_socket);
    }
    logger_.debug("Exiting RPCListenerThread");
}


Json Mediator::jsonUnpublishCallback(const Json &json) {
  return removePublisher(json["topic_name"].get<std::string>(), json["host"].get<std::string>(), json["port"].get<int>(), json["message_name"].get<std::string>());
}

Json Mediator::jsonPublishCallback(const Json &json) {
  return addPublisher(json["topic_name"].get<std::string>(), json["host"].get<std::string>(), json["port"].get<int>(), json["message_name"].get<std::string>());
}

Json Mediator::jsonUnsubscribeCallback(const Json &json) {
  return removeSubscriber(json["topic_name"].get<std::string>(), json["host"].get<std::string>(), json["port"].get<int>(), json["message_name"].get<std::string>());
}

Json Mediator::jsonSubscribeCallback(const Json &json) {
  return addSubscriber(json["topic_name"].get<std::string>(), json["host"].get<std::string>(), json["port"].get<int>(), json["message_name"].get<std::string>());
}

//TODO not sure if these are needed in this way
Json Mediator::addSubscriber(std::string const &topic_name, std::string const &host, int const port, std::string const& message_name) {
    LogContext context("Mediator::addSubscriber");
    logger_.debug("Adding subscriber to topic " + topic_name + " at " + host + ":" + std::to_string(port));

    Topic topic_info{host, port, topic_name, message_name};
    subMutex_.lock();
    // subscriberTable_[toURI(host, port)].emplace_back(topic_name);
    subscriberTable_[topic_name].push_back(topic_info);
    subMutex_.unlock();

    Json retVal = {{"code", 0}, {"status", "ready"}};

    pubMutex_.lock();
    //Get publishers
    pubMutex_.unlock();
    //TODO send to new subscriber the list of publishers

    return retVal;
}

Json Mediator::addPublisher(std::string const &topic_name, std::string const &host, int const port, std::string const& message_name) {
    LogContext context("Mediator::addPublisher");
    logger_.debug("Adding publisher to topic " + topic_name + " at " + host + ":" + std::to_string(port));

    Topic topic_info{host, port, topic_name, message_name};
    pubMutex_.lock();
    publisherTable_[topic_name].push_back(topic_info);
    pubMutex_.unlock();

    Json retVal = {{"code", 0}, {"status", "ready"}};

    subMutex_.lock();
    // std::vector<Topic> subscribers = subscriberTable_[topic_name];
    subMutex_.unlock();

    // for (auto &subscriber : subscribers) {
        //TODO send to subscriber the updated list of publishers
    // }
    return retVal;
}

Json Mediator::removeSubscriber(std::string const &topic_name, std::string const &host, int const port, std::string const& message_name) {
    LogContext context("Mediator::removeSubscriber");
    logger_.debug("Removing subscriber from topic " + topic_name + " at " + host + ":" + std::to_string(port));
    Topic topic_info{host, port, topic_name, message_name};
    subMutex_.lock();
    int dist = 0;
    auto it = subscriberTable_.find(topic_name);
    if (it != subscriberTable_.end()) {
      auto jt = std::remove_if(it->second.begin(), it->second.end(), [topic_info](Topic const& topic)->bool{
        return topic == topic_info;
      });
      dist = static_cast<int>(std::distance(jt, it->second.end()));
      it->second.erase(jt, it->second.end());
    } else {
      logger_.debug("Error: no such subscribers exist");
    }
    subMutex_.unlock();

    Json retVal = {{"code", 0}, {"status", "ready"}, {"numUnregistered", dist}};
    return retVal;
}

Json Mediator::removePublisher(std::string const &topic_name, std::string const &host, int const port, std::string const& message_name) {
    LogContext context("Mediator::removePublisher");
    logger_.debug("Removing publisher from topic " + topic_name + " at " + host + ":" + std::to_string(port));
    Topic topic_info{host, port, topic_name, message_name};
    pubMutex_.lock();
    int dist = 0;
    auto it = publisherTable_.find(topic_name);
    if (it != publisherTable_.end()) {
      auto jt = std::remove_if(it->second.begin(), it->second.end(), [topic_info](Topic const& topic)->bool {
        return topic == topic_info;
      });
      dist = static_cast<int>(std::distance(jt, it->second.end()));
      it->second.erase(jt, it->second.end());
    } else {
      logger_.debug("Error: no such publishers exist");
    }
    pubMutex_.unlock();

    subMutex_.lock();
    // std::vector<Topic> subscribers = subscriberTable_[topic_name];
    subMutex_.unlock();

    // for (auto &subscriber : subscribers) {
        //TODO send to subscriber the updated list of publishers
    // }
    Json retVal = {{"code", 0}, {"status", "ready"}, {"numUnregistered", dist}};
    return retVal;
}

void Mediator::addNode(std::string const &node_name, std::string const &host, int const port) {
    LogContext context("Mediator::addNode");
    logger_.debug("Adding node " + node_name + " at " + host + ":" + std::to_string(port));

    std::lock_guard<std::mutex> lock(nodeMutex_);
    nodeTable_[toURI(host, port)] = node_name;

    //TODO any calls to be made?
}

void Mediator::removeNode(std::string const &node_name, std::string const &host, int const port) {
    LogContext context("Mediator::removeNode");
    logger_.debug("Removing node " + node_name + " at " + host + ":" + std::to_string(port));

    std::lock_guard<std::mutex> lock(nodeMutex_);
    nodeTable_.erase(toURI(host, port));

    //TODO any calls to be made?
}


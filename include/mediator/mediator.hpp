#ifndef MROS_W24_SOLUTION_MAIN_NODE_HPP
#define MROS_W24_SOLUTION_MAIN_NODE_HPP

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <logging/logging.hpp>
#include <mros/mros.hpp>
#include <nlohmann/json.hpp>
#include <socket/bson_rpc_socket/bson_rpc_socket.hpp>
#include <socket/bson_rpc_socket/connection_bson_rpc_socket.hpp>
#include <socket/server_socket.hpp>
#include <thread>
#include <unordered_map>
#include <unordered_set>

using namespace std::chrono_literals;
using Json = nlohmann::json;



class Mediator : public std::enable_shared_from_this<Mediator> {

public:
    Mediator(std::string address, int port);

    Mediator();

    ~Mediator();

    void spin();

private:
    void RPCListenerThread();

    Json addSubscriber(std::string const &topic_name, std::string const &host, int const port, std::string const& message_name);

    Json addPublisher(std::string const &topic_name, std::string const &host, int const port, std::string const& message_name);

    Json removeSubscriber(std::string const &topic_name, std::string const &host, int const port, std::string const& message_name);

    Json removePublisher(std::string const &topic_name, std::string const &host, int const port, std::string const& message_name);

    void addNode(std::string const &node_name, std::string const &host, int const port);

    void removeNode(std::string const &node_name, std::string const &host, int const port);

    std::string address_;
    int port_;
    MROS& mros_;
    Logger &logger_;

    
    std::unique_ptr<ServerSocket> json_rpc_server_;

    struct Topic {
      std::string host;
      int port;
      std::string topic_name;
      std::string message_name;

      bool operator==(Topic const& rhs) const {
        return port = rhs.port && host == rhs.host && topic_name == rhs.topic_name && message_name == rhs.message_name;
      }
    };

    std::unordered_map<std::string, std::vector<Topic>> subscriberTable_;   // topic_name -> vector of topics
    std::unordered_map<std::string, std::vector<Topic>> publisherTable_;    // topic_name -> vector of topics
    std::unordered_map<std::string, std::string> nodeTable_;                //URI -> name

    std::unordered_set<std::shared_ptr<ConnectionBsonRPCSocket>> connection_list_;
    std::mutex subMutex_;
    std::mutex pubMutex_;
    std::mutex nodeMutex_;

    std::thread RPCListenerThread_;


    bool status() {
        return mros_.status();
    }

    Json jsonSubscribeCallback(Json const& json);

    Json jsonUnsubscribeCallback(Json const& json);

    Json jsonPublishCallback(Json const& json);

    Json jsonUnpublishCallback(Json const& json);

};



#endif //MROS_W24_SOLUTION_MAIN_NODE_HPP

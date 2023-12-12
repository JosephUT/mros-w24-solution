#ifndef MROS_W24_SOLUTION_MAIN_NODE_HPP
#define MROS_W24_SOLUTION_MAIN_NODE_HPP

#include <csignal>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <exception>
#include <thread>
#include <nlohmann/json.hpp>
#include "logging/logging.hpp"

using namespace std::chrono_literals;
using Json = nlohmann::json;

class MediatorSignalHandler {
public:

    friend class Mediator;

    MediatorSignalHandler(int argc, char** argv);

    ~MediatorSignalHandler() = default;

    void registerHandler();

    bool getStatus();


private:

    MediatorSignalHandler();

    void handleSignal(int signal);

    static void staticHandleSignal(int signal);

    void deactivateSignalHandler();

    Logger &logger_;
    static MediatorSignalHandler *instancePtr;
    std::atomic<bool> status{false};
};

class Mediator : public std::enable_shared_from_this<Mediator> {

public:
    Mediator(int argc, char** argv, std::string address, int port);

    Mediator(int argc, char** argv);

    Mediator() = delete;

    ~Mediator();

    void spin();

private:
    void RPCListenerThread();

    void addSubscriber(std::string const &topic_name, std::string const &host, int const port);

    void addPublisher(std::string const &topic_name, std::string const &host, int const port);

    void removeSubscriber(std::string const &topic_name, std::string const &host, int const port);

    void removePublisher(std::string const &topic_name, std::string const &host, int const port);

    void addNode(std::string const &node_name, std::string const &host, int const port);

    void removeNode(std::string const &node_name, std::string const &host, int const port);

    std::string address_;
    int port_;
    Logger &logger_;

    MediatorSignalHandler handler;
    
    int server_fd_;

    class Topic {
    public:
        Topic() = default;

        Topic(int sock_fd, std::string host, int port, std::string topic_name, std::string message_name) : sock_fd_(
                sock_fd), host_(std::move(host)), port_(port), topic_name_(std::move(topic_name)), message_name_(
                std::move(message_name)) {}

        Topic(Topic &&topic) noexcept = default;

        explicit Topic(Topic const &topic) {
            this->port_ = topic.port_;
            this->message_name_ = topic.message_name_;
            this->topic_name_ = topic.topic_name_;
            this->host_ = topic.host_;
        }


        bool operator==(Topic const &topic) const {
            return (this->port_ == topic.port_ && this->message_name_ == topic.message_name_ &&
                    this->topic_name_ == topic.topic_name_ && this->host_ == topic.host_);
        }

        Topic& operator=(Topic &&topic) noexcept = default;

        ~Topic() = default;

        int sock_fd() const {
            return sock_fd_;
        }

        std::string host() const {
            return host_;
        }

        int port() const {
            return port_;
        }

        std::string topic() const {
            return topic_name_;
        }

        std::string messageType() const {
            return message_name_;
        };

    private:
        // Socket information
        int sock_fd_;
        std::string host_;
        int port_;
        // Node information
        std::string topic_name_;
        std::string message_name_;

    };

    std::unordered_map<std::string, std::vector<Topic>> subscriberTable_;   //URI -> vector of topics
    std::unordered_map<std::string, std::vector<Topic>> publisherTable_;    //URI -> vector of topics
    std::unordered_map<std::string, std::string> nodeTable_;                //URI -> name

    std::mutex subMutex_;
    std::mutex pubMutex_;
    std::mutex nodeMutex_;

    std::thread RPCListenerThread_;


    bool status() {
        return handler.getStatus();
    }

    // void createMainSocket();

    // void startListening();

    // void jsonCallback(Json const &json, int const new_sock_fd);

    // bool publishCallback(Topic const &topic);

    // bool subscribeCallback(Topic const &topic);

    // bool unpublishCallback(Topic const &topic);

    // bool unsubscribeCallback(Topic const &topic);

    // bool createNodeCallback(std::string const &node_name);

    // bool bsonSender(const Json &json, int new_sock_fd);


};



#endif //MROS_W24_SOLUTION_MAIN_NODE_HPP

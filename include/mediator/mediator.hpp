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


/**
 * Signal handler for mediator. Similar to MROS
 */
class MediatorSignalHandler {
public:

    friend class Mediator;

    /**
     * Constructor for MediatorSignalHandler
     * @param argc number of command line arguments
     * @param argv command line arguments. Number denoted by argc
     */
    MediatorSignalHandler(int argc, char** argv);

    /**
     * Default dtor for MediatorSignalHandler
     */
    ~MediatorSignalHandler() = default;

    /**
     * Register signal handler for Mediator
     */
    void registerHandler();

    /**
     * Status of MediatorSignalHandler
     * @return boolean status controlled by SIGINT
     */
    bool getStatus();


private:

    /**
     * Base ctor hidden from public interface
     */
    MediatorSignalHandler();

    /**
     * Signal handler that controls status boolean
     * @param signal
     */
    void handleSignal(int signal);

    /**
     * Signal handler that is invoked in signal
     * @param signal type of signal received i.e. SIGINT
     */
    static void staticHandleSignal(int signal);

    /**
     * Sets status_ to false, allowing destruction of mediator
     */
    void deactivateSignalHandler();

    /**
     * Reference to logger
     */
    Logger &logger_;

    /**
     * Static pointer for MROS.
     */
    static MediatorSignalHandler *instancePtr;

    /**
     * Status controlled by signal handler
     */
    std::atomic<bool> status{false};
};

/**
 * Mediator that keeps a ledger of pubs and subs
 */
class Mediator : public std::enable_shared_from_this<Mediator> {
public:
    /**
     * Constructor of mediator that defines non-default address and port
     * @param argc number of command line arguments
     * @param argv command line arguments
     * @param address string address that is used
     * @param port port used
     */
    Mediator(int argc, char** argv, std::string address, int port);

    /**
     * Mediator ctor with loopback address and 13330 port
     * @param argc number of command line arguments
     * @param argv command line arguments
     */
    Mediator(int argc, char** argv);

    /**
     * Deleted base destructor of mediator
     */
    Mediator() = delete;

    /**
     * Mediator destructor
     * Cleans up threads and sends closing messages to pubs and subs
     */
    ~Mediator();

    /**
     * Handles incoming RPC calls initializes listener
     */
    void spin();

private:
    /**
     * Created listener thread for RPC calls
     */
    void RPCListenerThread();

    /**
     * Callback to add subscriber
     * @param topic_name name of topic to be subscribed
     * @param host socket host
     * @param port socket port
     */
    void addSubscriber(std::string const &topic_name, std::string const &host, int const port);

    /**
     * Callback to add publisher
     * @param topic_name name of topic to be published
     * @param host socket host
     * @param port socket port
     */
    void addPublisher(std::string const &topic_name, std::string const &host, int const port);

    /**
     * Callback to remove subscriber
     * @param topic_name name of topic to be unsubscribed
     * @param host socket host
     * @param port socket port
     */
    void removeSubscriber(std::string const &topic_name, std::string const &host, int const port);

    /** Callback to remove publisher
     * @param topic_name name of topic to be unpublished
     * @param host socket host
     * @param port socket port
     */
    void removePublisher(std::string const &topic_name, std::string const &host, int const port);

    /**
     * Callback to add node
     * @param node_name name of node
     * @param host socket host
     * @param port socket port
     */
    void addNode(std::string const &node_name, std::string const &host, int const port);

    /**
     * Callback to remove node
     * @param node_name name of node
     * @param host socket host
     * @param port socket port
     */
    void removeNode(std::string const &node_name, std::string const &host, int const port);

    /**
     * Address of mediator
     */
    std::string address_;

    /**
     * Mediator port
     */
    int port_;

    /**
     * Logger
     */
    Logger &logger_;

    /**
     * Signal handler
     */
    MediatorSignalHandler handler;

    /**
     * file descriptor for server
     */
    int server_fd_;

    /**
     * Data type that stores topic information. Allows comparison for all members except file descriptor for ease of use
     */
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

    /**
     * converts URI to topic for subscribers (uri example: http://127.0.0.1:8080/topic_name)
     */
    std::unordered_map<std::string, std::vector<Topic>> subscriberTable_;

    /**
     * Converts URI to topic for publishers
     */
    std::unordered_map<std::string, std::vector<Topic>> publisherTable_;

    /**
     * Converts URI to name of node
     */
    std::unordered_map<std::string, std::string> nodeTable_;

    /**
     * Mutex to control subscriberTable_
     */
    std::mutex subMutex_;

    /**
     * Mutex to control publisherTable_
     */
    std::mutex pubMutex_;

    /**
     * Mutex to control nodeTable_
     */
    std::mutex nodeMutex_;

    /**
     * Thread that allows for incoming connections and messages
     */
    std::thread RPCListenerThread_;

    /**
     * Accessed the signal handler's state
     * @return boolean controlled by SIGINT
     */
    bool status() {
        return handler.getStatus();
    }
};



#endif //MROS_W24_SOLUTION_MAIN_NODE_HPP

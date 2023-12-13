#ifndef MROS_W24_SOLUTION_RPC_SOCKET_HPP
#define MROS_W24_SOLUTION_RPC_SOCKET_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <socket/message_socket/message_socket.hpp>
#include <string>
#include <thread>
#include <unordered_map>

/**
 * String holding the name of a callback function to used to determine the appropriate callback to invoke.
 */
using CallbackName = std::string;

/**
 * Function taking a string message from the peer socket and returning nothing. Used for half duplex communication.
 */
using RequestCallback = std::function<void(std::string& callback_argument)>;

/**
 * Function taking a string message from the peer socket and returning a string message to send back to the peer socket.
 * Used for full duplex communication.
 */
using RequestResponseCallback = std::function<std::string(std::string& callback_argument)>;

/**
 * Function taking no arguments and returning nothing, using for handling closing routines that do not communicate.
 */
using ClosingCallback = std::function<void()>;

/**
 * Abstract mixin class to provide half and full duplex string based RPC to peer sockets, along with no a zero message
 * lose closing routine.
 */
class RPCSocket : virtual public MessageSocket {
 public:
  /**
   * Constructor setting is_connected_ to false.
   */
  RPCSocket();

  /**
   * Pure virtual destructor to force subclassing.
   */
  ~RPCSocket() override = 0;

  /**
   * Closes this socket and the connected socket using a three way handshake.
   */
  void close() override;

  /**
   * Check if the socket is connected.
   * @return True if the socket is connected, false otherwise.
   */
  bool connected();

  /**
   * Performs a half duplex RPC to the peer socket, invoking a callback a certain name with a supplied argument.
   * @param callback_name The name of the peer socket's callback to invoke.
   * @param callback_argument The string argument to pass to the peer socket's callback.
   */
  void sendRequest(const CallbackName& callback_name, const std::string& callback_argument);

  /**
   * Performs a full duplex RPC to the peer socket, invoking a callback in the peer socket which in turn invokes a
   * callback in this socket, passing the return of the previous call as the argument.
   * @param callback_name The name of the peer socket's callback to invoke.
   * @param callback_argument The string argument to pass to the peer socket's callback.
   * @param response_callback_name The callback of this socket to be invoked with the peer socket's return.
   */
  void sendRequestAndGetResponse(const CallbackName& callback_name, const std::string& callback_argument,
                                 const CallbackName& response_callback_name);

  /**
   * Adds a request callback to request_callbacks_ that can then be called by the peer socket.
   * @param callback_name The name of the callback, used as the key in request_callbacks_.
   * @param callback The callback function, used as the value in request_callbacks_.
   */
  void registerRequestCallback(const CallbackName& callback_name, const RequestCallback& callback);

  /**
   * Adds a request response callback to request_response_callbacks_ that can then be called by the peer socket.
   * @param callback_name The name of the callback, used as the key in request_response_callbacks_.
   * @param callback The callback function, used as the value in request_response_callbacks_.
   */
  void registerRequestResponseCallback(const CallbackName& callback_name, const RequestResponseCallback& callback);

  /**
   * Adds a request callback to request_callbacks_ with key kClosingCallbackName_. This callback will be executed by the
   * receiving thread once it receives a closing message.
   * @param callback Void routine taking no arguments to be executed immediately before closing the socket.
   */
  void registerClosingCallback(const ClosingCallback& callback);

 protected:
  /**
   * Remove sendMessage() from the public interface. Keep protected for use in this class and subclasses.
   */
  using MessageSocket::sendMessage;

  /**
   * Remove receiveMessage() from the public interface. Keep protected for use in this class and subclasses.
   */
  using MessageSocket::receiveMessage;

  /**
   * Run the receive cycle on the receiving thread and detach the receiving thread. Allows derived classes (clients and
   * connections) to being receiving at the appropriate time.
   */
  void startReceiveCycle();

  /**
   * Boolean that is true if the socket is connected, false otherwise.
   */
  std::atomic<bool> is_connected_;

 private:
  /**
   * Receive and decode messages, check for closing messages, and handle callbacks and return callbacks.
   */
  void receiveCycle();

  /**
   * Process a request by calling the appropriate callback.
   */
  void processRequest(const CallbackName& callback_name, std::string& callback_argument);

  /**
   * Process a request by calling the appropriate callback and sending the return request message.
   */
  void processRequestResponse(const CallbackName& callback_name, std::string& callback_argument,
                              const CallbackName& response_callback_name);

  /**
   * Send a closing message to the peer socket.
   */
  void sendClosingMessage();

  /**
   * Map for storing request callbacks.
   */
  std::unordered_map<CallbackName, RequestCallback> request_callbacks_;

  /**
   * Map for storing request response callbacks.
   */
  std::unordered_map<CallbackName, RequestResponseCallback> request_response_callbacks_;

  /**
   * Callback to be called if closing_callback_set_ is true when receiving thread receives a closing message.
   */
  ClosingCallback closing_callback_;

  /**
   * Boolean, true if closing_callback_ has been assigned, false otherwise. Guarded by closing_callback_lock_.
   */
  bool closing_callback_set_ = false;

  /**
   * Lock taken to ensure thread safety of accessing request_callbacks_.
   */
  std::mutex request_callbacks_lock_;

  /**
   * Lock taken to ensure thread safety of accessing request_response_callbacks_.
   */
  std::mutex request_response_callbacks_lock_;

  /**
   * Lock taken to ensure thread safety of accessing closing_callback_.
   */
  std::mutex closing_callback_lock_;

  /**
   * Internal thread to process messages and handle callbacks. It is detached during construction.
   */
  std::thread receiving_thread_;

  /**
   * Lock taken when sending to ensure thread safety of the sendMessage() function. Also leveraged in closing routine.
   */
  std::mutex sending_lock_;

  /**
   * Lock taken to ensure user thread waits until the receiving thread has received a closing message as part of the
   * three way handshake.
   */
  std::mutex closing_lock_;

  /**
   * Condition variable associated with the closing waiting condition. Used with closing_lock_.
   */
  std::condition_variable closing_condition_variable_;

  /**
   * Condition for if a closing message has been received. Waited on by closing_lock_ and closing_condition_variable_.
   */
  bool closing_message_received_ = false;

  /**
   * Variable for delimiting the parameters for callbacks within messages. One of these characters is sent as the
   * closing message.
   */
  static constexpr const char kParameterDelimiter_ = '%';
};

#endif  // MROS_W24_SOLUTION_RPC_SOCKET_HPP

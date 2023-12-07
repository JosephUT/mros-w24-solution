#include <gtest/gtest.h>

#include <thread>

#include "socket/rpc_socket/client_rpc_socket.hpp"
#include "socket/rpc_socket/connection_rpc_socket.hpp"
#include "socket/server_socket.hpp"

/**
 * Testing fixture for testing rpc sockets.
 */
class RPCSocketTest : public testing::Test {
 protected:
  /**
   * Initialize the server and client sockets.
   */
  void SetUp() {
    server_socket_ = std::make_shared<ServerSocket>(kDomain_, kServerAddress_, kServerPort_, kBacklogSize_);
    client_rpc_socket_ = std::make_shared<ClientRPCSocket>(kDomain_, kServerAddress_, kServerPort_);
  }

  /**
   * Close the server socket after testing. NOTE: Test cases must handle closing the client and connection sockets.
   */
  void TearDown() { server_socket_->close(); }

  /**
   * Shared pointers to three sockets for testing.
   */
  std::shared_ptr<ServerSocket> server_socket_;
  std::shared_ptr<ClientRPCSocket> client_rpc_socket_;
  std::shared_ptr<ConnectionRPCSocket> connection_rpc_socket_;

  /**
   * Setup parameters for sockets.
   */
  const int kDomain_ = AF_INET;
  const std::string kServerAddress_ = "127.0.0.1";
  const int kServerPort_ = 13335;
  const int kBacklogSize_ = 16;

 public:
  /**
   * Connect the client socket, retrying until success in case the server is not setup yet.
   */
  void connectClient() {
    while (true) {
      try {
        client_rpc_socket_->connectToServer();
        break;
      } catch (SocketException& error) {
      }
    }
  }

  /**
   * Accept a connection on the server, retrying until success in case the client is not setup yet.
   */
  void acceptConnection() {
    std::optional<std::shared_ptr<ConnectionRPCSocket>> optional_connection_rpc_socket;
    do {
      optional_connection_rpc_socket = server_socket_->acceptConnection<ConnectionRPCSocket>();
    } while (!optional_connection_rpc_socket);
    connection_rpc_socket_ = *optional_connection_rpc_socket;
  }

  /** TESTING FUNCTIONS **/
  /**
   * Client process for RequestConnectionSend test.
   */
  void requestConnectionSendClient() {
    RequestCallback callback = [this](std::string input)-> void { requestCallback1(input); };
    client_rpc_socket_->registerRequestCallback("requestCallback1", callback);
    connectClient();
  }

  /**
   * Server process for RequestConnectionSend test.
   */
  void requestConnectionSendServer() {
    acceptConnection();
    connection_rpc_socket_->startConnection();
    connection_rpc_socket_->sendRequest("requestCallback1", kRequestCallback1CorrectRequest_);
    connection_rpc_socket_->close();
  }

  /**
   * Client process for RequestClientSend test.
   */
  void requestClientSendClient() {
    connectClient();
    client_rpc_socket_->sendRequest("requestCallback1", kRequestCallback1CorrectRequest_);
    client_rpc_socket_->close();
  }

  /**
   * Server process for RequestClientSend test.
   */
  void requestClientSendServer() {
    acceptConnection();
    RequestCallback callback = [this](std::string input) -> void { requestCallback1(input); };
    connection_rpc_socket_->registerRequestCallback("requestCallback1", callback);
    connection_rpc_socket_->startConnection();
  }

  /**
   * Client process for RequestResponse test.
   */
  void requestResponseClient() {
    RequestResponseCallback callback = [this](std::string input) -> std::string { return requestResponseCallback1(std::move(input));};
    client_rpc_socket_->registerRequestResponseCallback("requestResponseCallback1", callback);
    connectClient();
  }

  /**
   * Server process for RequestResponse test.
   */
  void requestResponseServer() {
    acceptConnection();
    RequestCallback callback = [this](std::string input) -> void { requestCallback1(input); };
    connection_rpc_socket_->registerRequestCallback("requestCallback1", callback);
    connection_rpc_socket_->startConnection();
    connection_rpc_socket_->sendRequestAndGetResponse("requestResponseCallback1", kRequestCallback1CorrectRequest_, "requestCallback1");
    connection_rpc_socket_->close();
  }

  /**
   * Client process for ClosingCallback test.
   */
  void closingCallbackClient() {
    ClosingCallback callback = [this] () -> void {closingCallback1();};
    client_rpc_socket_->registerClosingCallback(callback);
    client_rpc_socket_->connectToServer();
  }

  /**
   * Server process for ClosingCallback test.
   */
  void closingCallbackServer() {
    acceptConnection();
    ClosingCallback callback = [this] () -> void {closingCallback1();};
    connection_rpc_socket_->registerClosingCallback(callback);
    connection_rpc_socket_->startConnection();
    connection_rpc_socket_->close();
  }

  /** TESTING CALLBACKS **/
  /**
   * Assert that the data sent to this callback is the same as the stored correct value and increment the call counter
   * for this function.
   */
  void requestCallback1(std::string& string) {
    ASSERT_EQ(string, kRequestCallback1CorrectRequest_);
    ++requestCallback1Count_;
  }
  const std::string kRequestCallback1CorrectRequest_ = "correct for request callback 1";

  /**
   * Call counter for the above function.
   */
  int requestCallback1Count_ = 0;

  /**
   * Return the input value and increment the call counter for this function.
   */
  std::string requestResponseCallback1(std::string string) {
    ++requestResponseCallback1Count_;
    return string;
  }

  /**
   * Call counter for the above function.
   */
  int requestResponseCallback1Count_ = 0;

  /**
   * Increment the counter for this function.
   */
  void closingCallback1() {
    ++closingCallback1Count_;
  }

  /**
   * Call counter for the above function.
   */
  int closingCallback1Count_ = 0;
};

/**
 * Test if a single request is handled correctly when sent from connection to client.
 */
TEST_F(RPCSocketTest, RequestConnectionSend) {
  std::thread client_thread(&RPCSocketTest::requestConnectionSendClient, this);
  std::thread server_thread(&RPCSocketTest::requestConnectionSendServer, this);
  client_thread.join();
  server_thread.join();
  ASSERT_EQ(requestCallback1Count_, 1);
}

/**
 * Test if a single request is handled correctly when sent from client to connection.
 *
 * NOTE: All test below will assume that connection to client and client to connection messaging behave identically and
 * will only test connection to client. This should be a safe assumption since all messaging functionality for both
 * connection and client is inherited from the same class (RPCSocket).
 */
TEST_F(RPCSocketTest, RequestClientSend) {
  std::thread client_thread(&RPCSocketTest::requestClientSendClient, this);
  std::thread server_thread(&RPCSocketTest::requestClientSendServer, this);
  client_thread.join();
  server_thread.join();
  ASSERT_EQ(requestCallback1Count_, 1);
}

/**
 * Test if a request response is handled correctly when sent from connection to client.
 */
TEST_F(RPCSocketTest, RequestResponse) {
  std::thread client_thread(&RPCSocketTest::requestResponseClient, this);
  std::thread server_thread(&RPCSocketTest::requestResponseServer, this);
  client_thread.join();
  server_thread.join();
  ASSERT_EQ(requestCallback1Count_, 1);
  ASSERT_EQ(requestResponseCallback1Count_, 1);
}

/**
 * Test if closing callbacks are called on close().
 */
TEST_F(RPCSocketTest, ClosingCallback) {
  std::thread client_thread(&RPCSocketTest::closingCallbackClient, this);
  std::thread server_thread(&RPCSocketTest::closingCallbackServer, this);
  client_thread.join();
  server_thread.join();
  ASSERT_EQ(closingCallback1Count_, 2);
}

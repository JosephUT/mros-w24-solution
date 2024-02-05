#include <gtest/gtest.h>

#include <thread>

#include "socket/bson_rpc_socket/client_bson_rpc_socket.hpp"
#include "socket/bson_rpc_socket/connection_bson_rpc_socket.hpp"
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
    // Set up complex Json message objects to send.
    kRequestCallback1CorrectRequest_["message"] = "correct for request callback 1";
    kRequestCallback1CorrectRequest_["ports"] = {12, 13, 14, 15, 16};
    kRequestCallback1CorrectRequest_["Node URI's"] = {"456t6h", "bghwifiut738", "bgyi7766u", "jbhi67879uo", "hji786wu"};
    kConnectingCallback1CorrectMessage_["message"] = "Node URI?";

    // Set up client and server sockets.
    server_socket_ = std::make_shared<ServerSocket>(kDomain_, kServerAddress_, kServerPort_, kBacklogSize_);
    client_rpc_socket_ = std::make_shared<ClientBsonRPCSocket>(kDomain_, kServerAddress_, kServerPort_);
  }

  /**
   * Close the server socket after testing. NOTE: Test cases must handle closing the client and connection sockets.
   */
  void TearDown() { server_socket_->close(); }

  /**
   * Shared pointers to three sockets for testing.
   */
  std::shared_ptr<ServerSocket> server_socket_;
  std::shared_ptr<ClientBsonRPCSocket> client_rpc_socket_;
  std::shared_ptr<ConnectionBsonRPCSocket> connection_rpc_socket_;

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
      } catch (SocketException &error) {
      }
    }
  }

  /**
   * Accept a connection on the server, retrying until success in case the client is not setup yet.
   */
  void acceptConnection() {
    while (!connection_rpc_socket_) {
      connection_rpc_socket_ = server_socket_->acceptConnection<ConnectionBsonRPCSocket>();
    }
  }

  /** TESTING FUNCTIONS **/
  /**
   * Client process for RequestConnectionSend test.
   */
  void requestConnectionSendClient() {
    RequestCallbackJson callback = [this](json const &input) -> void { requestCallback1(input); };
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
    RequestCallbackJson callback = [this](json const &input) -> void { requestCallback1(input); };
    connection_rpc_socket_->registerRequestCallback("requestCallback1", callback);
    connection_rpc_socket_->startConnection();
  }

  /**
   * Client process for RequestResponse test.
   */
  void requestResponseClient() {
    RequestResponseCallbackJson callback = [this](json const &input) -> json {
      return requestResponseCallback1(std::move(input));
    };
    client_rpc_socket_->registerRequestResponseCallback("requestResponseCallback1", callback);
    connectClient();
  }

  /**
   * Server process for RequestResponse test.
   */
  void requestResponseServer() {
    acceptConnection();
    RequestCallbackJson callback = [this](json const &input) -> void { requestCallback1(input); };
    connection_rpc_socket_->registerRequestCallback("requestCallback1", callback);
    connection_rpc_socket_->startConnection();
    connection_rpc_socket_->sendRequestAndGetResponse("requestResponseCallback1", kRequestCallback1CorrectRequest_,
                                                      "requestCallback1");
    connection_rpc_socket_->close();
  }

  /**
   * Client process for ConnectingCallback test.
   */
  void connectingCallbackClient() {
    client_rpc_socket_->connectToServer(kConnectingCallback1CorrectMessage_);
  }

  /**
   * Server process for ConnectingCallback test.
   */
  void connectingCallbackServer() {
    acceptConnection();
    RequestCallbackJson callback = [this](json const &input) -> void { connectingCallback1(input); };
    connection_rpc_socket_->registerConnectingCallback(callback);
    connection_rpc_socket_->startConnection();
    connection_rpc_socket_->close();
  }

  /**
   * Client process for ClosingCallback test.
   */
  void closingCallbackClient() {
    ClosingCallback callback = [this]() -> void { closingCallback1(); };
    client_rpc_socket_->registerClosingCallback(callback);
    client_rpc_socket_->connectToServer();
  }

  /**
   * Server process for ClosingCallback test.
   */
  void closingCallbackServer() {
    acceptConnection();
    ClosingCallback callback = [this]() -> void { closingCallback1(); };
    connection_rpc_socket_->registerClosingCallback(callback);
    connection_rpc_socket_->startConnection();
    connection_rpc_socket_->close();
  }

  /** TESTING CALLBACKS **/
  /**
   * Assert that the data sent to this callback is the same as the stored correct value and increment the call counter
   * for this function.
   */
  void requestCallback1(json const &message) {
    ASSERT_EQ(message, kRequestCallback1CorrectRequest_);
    ++requestCallback1Count_;
  }
  json kRequestCallback1CorrectRequest_;

  /**
   * Call counter for the above function.
   */
  int requestCallback1Count_ = 0;

  /**
   * Return the input value and increment the call counter for this function.
   */
  json requestResponseCallback1(json const &message) {
    ++requestResponseCallback1Count_;
    return message;
  }

  /**
   * Call counter for the above function.
   */
  int requestResponseCallback1Count_ = 0;

  /**
   * Increment the counter for this function.
   */
  void connectingCallback1(json const& message) {
    ASSERT_EQ(message, kConnectingCallback1CorrectMessage_);
    ++connectingCallback1Count_;
  }
  json kConnectingCallback1CorrectMessage_;

  /**
   * Call counter for the above function.
   */
  int connectingCallback1Count_ = 0;

  /**
   * Increment the counter for this function.
   */
  void closingCallback1() { ++closingCallback1Count_; }

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
TEST_F(RPCSocketTest, ConnectingCallback) {
  std::thread client_thread(&RPCSocketTest::connectingCallbackClient, this);
  std::thread server_thread(&RPCSocketTest::connectingCallbackServer, this);
  client_thread.join();
  server_thread.join();
  ASSERT_EQ(connectingCallback1Count_, 1);
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

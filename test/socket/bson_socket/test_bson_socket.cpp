#include <gtest/gtest.h>

#include <string>
#include <iostream>

#include "socket/bson_socket/client_bson_socket.hpp"
#include "socket/bson_socket/connection_bson_socket.hpp"
#include "socket/server_socket.hpp"

/**
 * Testing fixture for the Bson family of socket classes (server, client, and connection sockets).
 */
class MessageSocketTest : public testing::Test {
 protected:
  /**
   * Set up pointers to a trio of a server and two connected sockets.
   */
  void SetUp() override {
    // Set up test messages.
    message1_["message"] = "sending a test string";
    message2_["message"] = "another test string for testing if multiple receives are different";
    message3_["message"] = "a final test string just to be sure";
    bad_message_["message"] = "sending a test string containing a $ character";
    long_message_["message"] = std::string(100000, 'a');

    // Set up connection socket.
    server_socket_ = std::make_unique<ServerSocket>(kDomain_, kServerAddress_, kServerPort_, kBacklogSize_);
    client_socket_ = std::make_unique<ClientBsonMessageSocket>(kDomain_, kServerAddress_, kServerPort_);
    client_socket_->connect();
    while (!connection_socket_){
      connection_socket_ = server_socket_->acceptConnection<ConnectionBsonSocket>();
    }
  }

  /**
   * Close the sockets.
   */
  void TearDown() override {
    server_socket_->close();
    client_socket_->close();
    connection_socket_->close();
  }

  /**
   * Shared pointers to server, client, and connection socket variables.
   */
  std::unique_ptr<ServerSocket> server_socket_;
  std::unique_ptr<ClientBsonMessageSocket> client_socket_;
  std::shared_ptr<ConnectionBsonSocket> connection_socket_;

  /**
   * Strings containing valid test messages.
   */
  json message1_;
  json message2_;
  json message3_;

  /**
   * String containing an invalid delimiting character. NOTE: Changing the choice of delimiting character will cause
   * this test to fail.
   */
  json bad_message_;

  /**
   * Length, character, and corresponding large string to stress test send and receive.
   */
  json long_message_;

  /**
   * Setup parameters for sockets.
   */
  const int kDomain_ = AF_INET;
  const std::string kServerAddress_ = "127.0.0.1";
  const int kServerPort_ = 13330;
  const int kBacklogSize_ = 1;
};

/**
 * Test if a single send and receive in each direction are valid.
 */
TEST_F(MessageSocketTest, SendReceive) {
  connection_socket_->sendMessage(message1_);
  client_socket_->sendMessage(message1_);
  ASSERT_EQ(client_socket_->receiveMessage(), message1_) << "Client socket does not receive from connection socket.";
  ASSERT_EQ(connection_socket_->receiveMessage(), message1_)
      << "Connection socket does not receive from client socket.";
}

/**
 * Test if multiple sends are properly received and separated.
 */
 TEST_F(MessageSocketTest, MultipleSendSingleReceive) {
  std::vector<json> messages = {message1_, message2_, message3_};
  for (auto const& message : messages) {
    connection_socket_->sendMessage(message);
    client_socket_->sendMessage(message);
  }
  for (auto const& message : messages) {
    ASSERT_EQ(connection_socket_->receiveMessage(), message);
    ASSERT_EQ(client_socket_->receiveMessage(), message);
  }
}
//
///**
// * Test if the sockets can handle large messages.
// */
// TEST_F(MessageSocketTest, StressSendReceive) {
//  connection_socket_->sendMessage(long_message_);
//  client_socket_->sendMessage(long_message_);
//  ASSERT_EQ(connection_socket_->receiveMessage(), long_message_);
//  ASSERT_EQ(client_socket_->receiveMessage(), long_message_);
//}
//
///**
// * Test if sockets can be reinitialized and communicated over.
// */
// TEST_F(MessageSocketTest, CloseReopen) {
//  TearDown();
//  server_socket_.reset();
//  connection_socket_.reset();
//  client_socket_.reset();
//  SetUp();
//  connection_socket_->sendMessage(message1_);
//  client_socket_->sendMessage(message1_);
//  ASSERT_EQ(client_socket_->receiveMessage(), message1_);
//  ASSERT_EQ(connection_socket_->receiveMessage(), message1_);
//}
//
///**
// * Test if sockets throw when send or receive is called on a closed socket.
// */
// TEST_F(MessageSocketTest, ThrowClosed) {
//  client_socket_->close();
//  ASSERT_THROW(client_socket_->sendMessage(message1_), SocketException);
//  ASSERT_THROW(client_socket_->receiveMessage(), SocketException);
//  ASSERT_THROW(connection_socket_->sendMessage(message1_), PeerClosedException);
//  ASSERT_THROW(connection_socket_->receiveMessage(), PeerClosedException);
//}
//
///**
// * Test if sockets throw when send is called with a message that contains a delimiting character.
// */
// TEST_F(MessageSocketTest, ThrowMessageContainsDelimiter) {
//  ASSERT_THROW(connection_socket_->sendMessage(bad_message_), SocketException);
//  ASSERT_THROW(client_socket_->sendMessage(bad_message_), SocketException);
//}

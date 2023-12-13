#ifndef MROS_W24_SOLUTION_SOCKET_EXCEPTION_HPP
#define MROS_W24_SOLUTION_SOCKET_EXCEPTION_HPP

#include <stdexcept>
#include <string>

/**
 * Runtime error to throw on socket failures that should be handled at runtime.
 */
class SocketException : public std::runtime_error {
 public:
  /**
   * Constructor to produce runtime error that will throw an informative error message.
   * @param error_message Error message detailing context of error.
   */
  explicit SocketException(const std::string& error_message)
      : std::runtime_error("SocketException: " + error_message) {}
};

#endif  // MROS_W24_SOLUTION_SOCKET_EXCEPTION_HPP

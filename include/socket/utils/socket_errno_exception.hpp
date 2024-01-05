#ifndef MROS_SOCKET_ERRNO_EXCEPTION_HPP
#define MROS_SOCKET_ERRNO_EXCEPTION_HPP

#include <cstring>
#include <cerrno>
#include <socket/utils/socket_exception.hpp>

/**
 * Runtime error to throw for socket failures that set the errno error code macro.
 */
class SocketErrnoException : public SocketException {
 public:
  /**
   * Constructor to add errno code to the error message.
   * @param error_message Error message detailing context of error.
   */
  explicit SocketErrnoException(const std::string& error_message)
      : SocketException(error_message + " Error: " + strerror(errno)) {}
};

#endif  // MROS_SOCKET_ERRNO_EXCEPTION_HPP

#ifndef MROS_W24_SOLUTION_LOGGING_HPP
#define MROS_W24_SOLUTION_LOGGING_HPP
#include <log4cxx/logger.h>
#include <log4cxx/ndc.h>
#include <memory>

#pragma once

//////////////////////////////////////////////////////////////////////////
// This class provides a handy wrapper around common logging activities //
//////////////////////////////////////////////////////////////////////////

/**
 * @brief Simple wrapper around the Nested Diagnostic Context class
 *
 * When declared at the beginning of a C++ scope, all logging calls
 * within that context will be prefixed with the name.
 *
 * Example:
 * void foo()
 * {
 *   LogContext ctx0("foo()");
 *   logger_.info("Welcome!");
 *   if (true)
 *   {
 *      LogContext ctx1("inner");
 *      logger_.info("True");
 *   }
 *   logger_.info("Goodbye!");
 * }
 *
 * Output:
 * [INFO          0 | demo foo()] Welcome!
 * [INFO          0 | demo foo() inner] True
 * [INFO          0 | demo foo()] Goodbye!
 */
class LogContext : public log4cxx::NDC
{
public:
  LogContext(const std::string& name) : NDC(name)
  {
  }
};

/**
 * @brief Simple logger wrapper
 *
 * There are three main logging methods:
 *   * warn
 *   * info
 *   * debug
 *
 * Debug is only printed when a -v flag is passed on the command line.
 * The formatting of messages is
 * [INFO          0 | demo foo()] Welcome!
 * where
 *  INFO is the logging level used
 *  0 is the number of milliseconds since execution began
 *  demo is the root context name
 *  foo() is all the additional contexts (as needed)
 *  Welcome! is the message passed to the logging method.
 *
 * Needs to be initialized in the main method with
 * Logger& logger = Logger::getLogger();
 * logger.initialize(argc, argv, "demo");
 *
 * This will initialize the standard configuration with the provided string
 * as the root context name. The arguments are provided to check for the -v flag.
 */
class Logger
{
public:
  /// Singleton access
  static Logger& getLogger()
  {
    static Logger instance;
    return instance;
  }

  /// Remove constructor for singleton
  Logger(Logger const&) = delete;
  /// Remove operator for singleton
  void operator=(Logger const&) = delete;

  /**
   * @brief Initialize the logger
   * @param argc Number of arguments on the command line
   * @param argv The value of the arguments on the command line
   * @param name The root context name
   */
  void initialize(int argc, char* argv[], const std::string& name);

  template <typename... Args>
  void debug(Args... args) const
  {
    LOG4CXX_DEBUG(root_, args...);
  }

  template <typename... Args>
  void info(Args... args) const
  {
    LOG4CXX_INFO(root_, args...);
  }

  template <typename... Args>
  void warn(Args... args) const
  {
    LOG4CXX_WARN(root_, args...);
  }

protected:
  log4cxx::LoggerPtr root_;
  std::unique_ptr<LogContext> context_;

private:
  Logger();
};

#endif //MROS_W24_SOLUTION_LOGGING_HPP

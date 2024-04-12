#pragma once

#include <atomic>
#include <csignal>
#include <functional>
#include <memory>

#include "logging/logging.hpp"

class MROS {
 public:
  ~MROS();

  /**
   * Set up the single instance of the class in this process. This must be called before any other functionality can
   * be used.
   */
  static void init(int argc, char** argv);

  /**
   * Get a reference to the single instance of the class in this process.
   */
  static MROS& getMROS();

  /**
   * Check if the instance has been set up and ctrl+C has not yet been pressed.
   */
  bool active();

  /**
   * Register an additional routine to be executed on ctrl+C. By default ctrl+C will simply call set active_ to false.
   */
  void registerDeactivateRoutine(std::function<void(void)> const& function);

  /**
   * Deleted copy constructor for singleton class.
   */
  MROS(MROS const& other) = delete;

  /**
   * Deleted assignment operator for singleton class.
   */
  void operator=(MROS const& other) = delete;

 private:
  MROS() = default;
  MROS(int argc, char** argv);

  /**
   * Perform the deactivation routine if the instance has been set up. Static function to be registered as the ctrl+C
   * (SIGINT) callback.
   */
  static void staticHandleSignal(int signal);

  /**
   * Deactivate the instance and run every registered deactivate_routine in the order they were registered. Called by
   * staticHandleSignal().
   */
  void deactivate();

  static MROS* mros_ptr_;
  std::atomic<bool> active_;
  std::vector<std::function<void(void)>> deactivate_routines_;

  Logger& logger_;
};

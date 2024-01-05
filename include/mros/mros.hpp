#ifndef MROS_W24_SOLUTION_MROS_HPP
#define MROS_W24_SOLUTION_MROS_HPP

#include <memory>
#include <atomic>
#include <csignal>
#include "logging/logging.hpp"

/**
 * MROS class that handles signals and logger
 */
class MROS {
public:
    /**
     * Destructor for MROS singleton
     */
    ~MROS();

    /**
     * Static initializer for MROS. Determines whether logger is in debugging mode or etc
     * @param argc number of arguments from command line
     * @param argv command line arguments. Number determined by argc
     */
    static void init(int argc, char** argv);

    /**
     * Gives access to MROS as a signal handler and the logger
     * @return Reference to static MROS object if and only if MROS::init has been called prior
     */
    static MROS& getMROS();

    /**
     * Access logger through MROS
     * @return reference to logger, as defined in logging
     */
    Logger& getLogger();

    /**
     * State variable for overriden signal handler
     * @return boolean denoting state of MROS
     */
    bool status();

    /**
     * Public interface for signal handler
     */
    void registerHandler();

    /**
     * Removes copy constructor for MROS. Move only
     * @param mros
     */
    MROS(MROS const& mros) = delete;

    /**
     * Removes assignment operator for MROS. Move only
     * @param mros
     */
    void operator=(MROS const& mros) = delete;
protected:
    /**
     * MROS object pointer. Static for use in signal handler
     */
    static MROS* mros_ptr_;

    /**
     * Reference to Logger
     */
    Logger& logger_;

    /**
     * Boolean for whether MROS::init has been called prior to fetching logger reference
     */
    bool check_logger_;

    /**
     * Thread-safe (atomic) boolean controlled by SIGINT. Used for determining when to kill node
     */
    std::atomic_bool status_;
private:
    /**
     * Base constructor removed from public interface.
     */
    MROS();

    /**
     * Constructor to create static mros pointer. Initializes MROS privately
     * @param argc number of command line arguments
     * @param argv command line arguments, where number is given by argc
     */
    MROS(int argc, char** argv);

    /**
     * Signal handler invoked by staticHandleSignal that changes status_ variable
     * @param signal required parameter that denotes signal type i.e. SIGINT
     */
    void handleSignal(int signal);

    /**
     * Static signal handler for use in registerHandler
     * @param signal required parameter that denotes signal type i.e. SIGINT
     */
    static void staticHandleSignal(int signal);

    /**
     * invoked by static signal handler if SIGINT is received. Changes status_ to false
     */
    void deactivateSignal();
};

#endif //MROS_W24_SOLUTION_MROS_HPP

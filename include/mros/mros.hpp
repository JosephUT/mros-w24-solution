#ifndef MROS_W24_SOLUTION_MROS_HPP
#define MROS_W24_SOLUTION_MROS_HPP

#include <memory>
#include <atomic>
#include <csignal>
#include "logging/logging.hpp"

class MROS {
public:
    ~MROS();

    static void init(int argc, char** argv);

    static MROS& getMROS();

    Logger& getLogger();

    bool status();

    void spin();

    MROS(MROS const& mr) = delete;

    void operator=(MROS const& mros) = delete;
protected:
    static MROS* mros_ptr_;
    Logger& logger_;
    bool check_logger_;
    std::atomic_bool status_;
private:
    MROS();

    MROS(int argc, char** argv);

    void handleSignal(int signal);

    static void staticHandleSignal(int signal);

    void deactivateSignal();
};

#endif //MROS_W24_SOLUTION_MROS_HPP

#include <logging/logging.hpp>
#include <string>


int main(int argc, char** argv) {
    Logger& logger = Logger::getLogger();
    logger.initialize(argc, argv, "LoggerTest");

    logger.info("Hello");
    logger.debug("Test");

    return 0;
}
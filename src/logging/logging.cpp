#include "logging/logging.hpp"
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <memory>

Logger::Logger()
{
  log4cxx::BasicConfigurator::configure();
  log4cxx::LayoutPtr p(new log4cxx::PatternLayout("[%-5p %09r | %x] %m%n"));
  root_ = log4cxx::Logger::getRootLogger();

  for (auto& appender : root_->getAllAppenders())
  {
    appender->setLayout(p);
  }

  root_->setLevel(log4cxx::Level::getInfo());
}

void Logger::initialize(int argc, char* argv[], const std::string& name)
{
  for (int i = 1; i < argc; i++)
  {
    std::string arg = argv[i];
    if (arg == "-v")
    {
      root_->setLevel(log4cxx::Level::getDebug());
    }
  }
  context_ = std::make_unique<LogContext>(name);
}

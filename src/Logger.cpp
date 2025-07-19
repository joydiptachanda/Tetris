#include "Logger.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>

#define VERBOSE_LOGGING 1

Logger::Logger() : logfile("tetris.log", std::ios::app) {}

Logger &Logger::getInstance()
{
    static Logger instance;
    return instance;
}

std::string Logger::getTimeStr()
{
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << "[" << std::put_time(&tm, "%H:%M:%S") << "] ";
    return oss.str();
}

void Logger::log(const std::string &msg)
{
#if VERBOSE_LOGGING
    if (logfile.is_open())
        logfile << getTimeStr() << msg << std::endl;
#endif
}

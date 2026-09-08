#include "Logger.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>

static constexpr bool kVerboseLogging = true;

Logger::Logger() : logfile("tetris.log", std::ios::app) {}

Logger &Logger::getInstance()
{
    static Logger instance;
    return instance;
}

std::string Logger::getTimeStr()
{
    auto t = std::time(nullptr);
    std::tm tm{};
    localtime_r(&t, &tm);
    std::ostringstream oss;
    oss << "[" << std::put_time(&tm, "%H:%M:%S") << "] ";
    return oss.str();
}

void Logger::log(const std::string &msg)
{
    if (kVerboseLogging && logfile.is_open())
        logfile << getTimeStr() << msg << std::endl;
}

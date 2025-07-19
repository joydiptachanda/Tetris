#pragma once
#include <string>
#include <fstream>

class Logger
{
public:
    static Logger &getInstance();
    void log(const std::string &msg);

private:
    Logger();
    std::ofstream logfile;
    static std::string getTimeStr();
};

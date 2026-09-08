// =============================================================================
// Logger.hpp
// -----------------------------------------------------------------------------
// Simple timestamped file logger (tetris.log), exposed as a singleton.
// =============================================================================
#pragma once
#include <string>
#include <fstream>

/**
 * @brief Simple timestamped file logger.
 *
 * @details
 * Writes to tetris.log, exposed as a process-wide singleton.
 */
class Logger
{
public:
    /** @brief Returns the process-wide Logger instance, creating it on first use. */
    static Logger &getInstance();

    /**
     * @brief Appends a timestamped line to the log file.
     * @param [in] msg Message to log.
     */
    void log(const std::string &msg);

private:
    Logger();
    std::ofstream logfile;
    /** @brief Returns the current time formatted as "[HH:MM:SS] ". */
    static std::string getTimeStr();
};

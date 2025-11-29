#include "Logger.h"
#include <iostream>

namespace ArkBeacon
{
    // ANSI color code definitions
    const std::string Logger::COLOR_RESET = "\033[0m";
    const std::string Logger::COLOR_GREEN = "\033[32m";    // Debug
    const std::string Logger::COLOR_BLUE = "\033[34m";     // Info
    const std::string Logger::COLOR_YELLOW = "\033[33m";   // Warning
    const std::string Logger::COLOR_RED = "\033[31m";      // Error

    const std::string Logger::m_interactive_console_line = "Enter command (type 'help' for available commands): ";

    std::string Logger::GetColorForLevel(LogLevel level)
    {
        switch (level)
        {
            case LogLevelDebug:
                return COLOR_GREEN;
            case LogLevelInfo:
                return COLOR_BLUE;
            case LogLevelWarning:
                return COLOR_YELLOW;
            case LogLevelError:
                return COLOR_RED;
            default:
                return COLOR_RESET;
        }
    }

    void Logger::Log(LogLevel level, const std::string& message)
    {
        std::string level_str;
        std::string color = GetColorForLevel(level);

        switch (level)
        {
            case LogLevelDebug:
                level_str = "[DEBUG]";
                break;
            case LogLevelInfo:
                level_str = "[INFO]";
                break;
            case LogLevelWarning:
                level_str = "[WARNING]";
                break;
            case LogLevelError:
                level_str = "[ERROR]";
                break;
        }

        // Print the log message with color
        std::cout << color << level_str << " " << message << COLOR_RESET << std::endl;
    }

    void Logger::PrintInteractiveConsoleLine()
    {
        std::cout << m_interactive_console_line << std::flush;
    }
}

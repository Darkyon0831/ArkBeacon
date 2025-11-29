#ifndef __ARKBEACON_LOGGER_H__
#define __ARKBEACON_LOGGER_H__

#include <string>

namespace ArkBeacon
{
    class Logger
    {
    public:
        enum LogLevel
        {
            LogLevelDebug,
            LogLevelInfo,
            LogLevelWarning,
            LogLevelError
        };

        static void Log(LogLevel level, const std::string& message);
        static void PrintInteractiveConsoleLine();

    private:
        // ANSI color codes
        static const std::string COLOR_RESET;
        static const std::string COLOR_GREEN;    // Debug
        static const std::string COLOR_BLUE;     // Info
        static const std::string COLOR_YELLOW;   // Warning
        static const std::string COLOR_RED;      // Error

        static const std::string m_interactive_console_line;
        static std::string GetColorForLevel(LogLevel level);
    };
}

#endif
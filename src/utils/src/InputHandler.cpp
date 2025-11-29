#include "InputHandler.h"
#include "Logger.h"

void ArkBeacon::InputHandler::StartInputLoop()
{
    std::string given_command;

    if (m_before_input_callback)
            m_before_input_callback();

    ArkBeacon::Logger::PrintInteractiveConsoleLine();

    while (m_running)
    {
        std::getline(std::cin, given_command);

        if (!m_input_enabled)
        {
            continue;
        }

        for (const auto& command : m_commands)
        {   
            std::string command_name = std::string(command.name);

            if (command_name == given_command)
            {
                command.action();
                break;
            }
        }
    }
}

void ArkBeacon::InputHandler::StopInput()
{
    m_running = false;
}

void ArkBeacon::InputHandler::DisableInput()
{
    m_input_enabled = false;
}

void ArkBeacon::InputHandler::EnableInput()
{
    m_input_enabled = true;
}

void ArkBeacon::InputHandler::PrintDescription() const
{
    ArkBeacon::Logger::Log(ArkBeacon::Logger::LogLevelInfo, "Available commands:");
    for (const auto& command : m_commands)
    {
        ArkBeacon::Logger::Log(ArkBeacon::Logger::LogLevelInfo, std::string(" - ") + std::string(command.name) + ": " + std::string(command.description));
    }

    ArkBeacon::Logger::PrintInteractiveConsoleLine();
}

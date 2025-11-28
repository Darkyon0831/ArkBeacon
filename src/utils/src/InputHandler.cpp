#include "InputHandler.h"

void ArkBeacon::InputHandler::StartInputLoop()
{
    std::string given_command;

    if (m_before_input_callback)
            m_before_input_callback();

    std::cout << "Enter command: ";

    while (m_running)
    {
        std::getline(std::cin, given_command);

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

void ArkBeacon::InputHandler::PrintDescription() const
{
    std::cout << "Available commands:" << std::endl;
    for (const auto& command : m_commands)
    {
        std::cout << " - " << command.name << ": " << command.description << std::endl;
    }
}

#ifndef __INPUTHANDLER_H__
#define __INPUTHANDLER_H__

#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <functional>

namespace ArkBeacon
{
    class InputHandler
    {
    public:
        struct Command
        {
            std::function<void()> action;
            std::string_view name;
            std::string_view description;
        };

        InputHandler() : m_running(true) {};
        ~InputHandler() = default;

        void StartInputLoop();

        void AddCommand(Command&& command) { m_commands.push_back(std::move(command)); }
        void AddCommand(const Command& command) { m_commands.push_back(command); }

        void StopInput();

        void SetBeforeInputCallback(std::function<void()> before_input_callback) { m_before_input_callback = before_input_callback; }

        void PrintDescription() const;

    private:
        bool m_running;
        std::vector<Command> m_commands;

        std::function<void()> m_before_input_callback = nullptr;
    };
}   

#endif // __INPUTHANDLER_H__
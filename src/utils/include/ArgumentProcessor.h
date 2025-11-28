#ifndef __ARKBEACON_COMMANDLINEARGUMENTS_H__
#define __ARKBEACON_COMMANDLINEARGUMENTS_H__

#include <string>
#include <vector>
#include <unordered_map>
#include <format>
#include <iostream>

namespace ArkBeacon
{
    class ArgumentProcessor
    {
    public:
        ArgumentProcessor();

        struct Argument
        {
            std::string name;
            std::vector<std::string> values;
        };

        bool HandleArguments(int argc, char* argv[]);
        const Argument* GetArgument(std::string_view name) const;
        const Argument* GetDefault(std::string_view name) const;
        void AddArgument(std::string name, std::vector<std::string>&& values);
        void PrintError(std::string_view message) const;
        void AddArgumentDefiner(std::string name, int num_values);
        void AddDefaultArgument(std::string name, std::vector<std::string>&& values);

    private:
        std::vector<Argument> m_arguments;
        std::unordered_map<std::string, int> m_argument_definer;
        std::vector<Argument> m_defaults;
    };
}

#endif
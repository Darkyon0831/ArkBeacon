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

        struct ArgumentDefiner
        {
            std::string name;
            std::vector<std::string_view> sub_arguments;
        };

        bool HandleArguments(int argc, char* argv[]);
        const Argument* GetArgument(std::string_view name) const;
        const Argument* GetDefault(std::string_view name) const;
        void AddArgument(std::string name, std::vector<std::string>&& values);
        void PrintError(std::string_view message) const;
        void AddArgumentDefiner(std::string name, std::vector<std::string_view>&& sub_arguments);
        void AddDefaultArgument(std::string name, std::vector<std::string>&& values);

    private:
        std::vector<Argument> m_arguments;
        std::vector<ArgumentDefiner> m_argument_definer;
        std::vector<Argument> m_defaults;
    };
}

#endif
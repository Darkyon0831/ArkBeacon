#include "ArgumentProcessor.h"

namespace ArkBeacon
{
    ArgumentProcessor::ArgumentProcessor()
    {
        
    }

    bool ArgumentProcessor::HandleArguments(int argc, char* argv[])
    {
        m_arguments.clear();
        int i = 1;
        while (i < argc)
        {
            Argument argument;
            std::string full_name = argv[i];
            argument.name = full_name.substr(1, full_name.size() - 1);
            int num_values = 0;

            bool found = false;
            for (auto& argument_definer : m_argument_definer)
            {
                if (argument_definer.first == argument.name)
                {
                    num_values = argument_definer.second;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                PrintError(std::format("Invalid argument {0}", argument.name));
                return false;
            }

            for (int j = 0; j < num_values; ++j)
            {
                ++i;
                if (i < argc)
                    argument.values.push_back(argv[i]);
                else
                {
                    PrintError(std::format("Invalid number of values in argument {0}, must have {1} values", argument.name, num_values));
                    return false;
                }
            }

            #ifdef DEBUG
            if (argument.values.empty() == false)
            {
                std::cout << "Added argument: " << argument.name << " with values: ";
                for (const auto& value : argument.values)
                {
                    std::cout << value << " ";
                }
                std::cout << std::endl;
            }
            #endif

            if (found)
                m_arguments.push_back(std::move(argument));

            ++i;
        }

        return true;
    }

    const ArgumentProcessor::Argument* ArgumentProcessor::GetDefault(std::string_view name) const
    {
        for (const auto& def : m_defaults)
        {
            if (def.name == name)
                return &def;
        }
        return nullptr;
    }

    void ArgumentProcessor::AddArgument(std::string name, std::vector<std::string> &&values)
    {
        Argument arg;
        arg.name = name;
        arg.values = std::move(values);
        m_arguments.push_back(std::move(arg));
        #ifdef DEBUG
        std::cout << "Added argument: " << arg.name << " with values: ";
        for (const auto& value : arg.values)
        {            
            std::cout << value << " ";
        }
        std::cout << std::endl; // Debug output
        #endif
    }

    void ArgumentProcessor::PrintError(std::string_view message) const
    {
        std::cerr << message << std::endl;
    }

    const ArgumentProcessor::Argument* ArgumentProcessor::GetArgument(std::string_view name) const
    {
        for (const auto& arg : m_arguments)
        {
            #ifdef DEBUG
            std::cout << "Checking argument: " << arg.name << std::endl;
            #endif
            if (arg.name == std::string(name))
            {
                #ifdef DEBUG
                std::cout << "Found argument: " << arg.name << " with values: ";
                for (const auto& value : arg.values)
                {
                    std::cout << value << " ";
                }
                std::cout << std::endl;
                #endif
                return &arg;
            }
        }

        if (const Argument* def = GetDefault(name))
            return def;

        return nullptr; 
    }

    void ArgumentProcessor::AddArgumentDefiner(std::string name, int num_values)
    {
        m_argument_definer.insert({name, num_values});

        int i = 0;
    }

    void ArgumentProcessor::AddDefaultArgument(std::string name, std::vector<std::string>&& values)
    {
        Argument arg;
        arg.name = name;
        arg.values = std::move(values);
        m_defaults.push_back(std::move(arg));
    }
}
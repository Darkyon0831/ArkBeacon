#include "ArgumentProcessor.h"
#include "Logger.h"

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
                if (argument_definer.name == argument.name)
                {
                    num_values = argument_definer.sub_arguments.size();
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
                    std::string string_builder = std::format("Not corrent values for argument {0}. Expected values for argument {0} are: ", argument.name);

                    for (const auto& sub_arg : m_argument_definer)
                    {
                        if (sub_arg.name == argument.name)
                        {
                            for (const auto& sub_argument_name : sub_arg.sub_arguments)
                            {
                                string_builder += std::format("<{0}> ", sub_argument_name);
                            }
                            break;
                        }
                    }

                    PrintError(string_builder);
                    return false;
                }
            }

            #ifdef DEBUG
            if (argument.values.empty() == false)
            {
                std::string msg = "Added argument: " + argument.name + " with values: ";
                for (const auto& value : argument.values)
                {
                    msg += value + " ";
                }
                ArkBeacon::Logger::Log(ArkBeacon::Logger::LogLevelDebug, msg);
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
        std::string msg = "Added argument: " + arg.name + " with values: ";
        for (const auto& value : arg.values)
        {            
            msg += value + " ";
        }
        ArkBeacon::Logger::Log(ArkBeacon::Logger::LogLevelDebug, msg);
        #endif
    }

    void ArgumentProcessor::PrintError(std::string_view message) const
    {
        ArkBeacon::Logger::Log(ArkBeacon::Logger::LogLevelError, std::string(message));
    }

    const ArgumentProcessor::Argument* ArgumentProcessor::GetArgument(std::string_view name) const
    {
        for (const auto& arg : m_arguments)
        {
            #ifdef DEBUG
            ArkBeacon::Logger::Log(ArkBeacon::Logger::LogLevelDebug, "Checking argument: " + arg.name);
            #endif
            if (arg.name == std::string(name))
            {
                #ifdef DEBUG
                std::string msg = "Found argument: " + arg.name + " with values: ";
                for (const auto& value : arg.values)
                {
                    msg += value + " ";
                }
                ArkBeacon::Logger::Log(ArkBeacon::Logger::LogLevelDebug, msg);
                #endif
                return &arg;
            }
        }

        if (const Argument* def = GetDefault(name))
            return def;

        return nullptr; 
    }

    void ArgumentProcessor::AddArgumentDefiner(std::string name, std::vector<std::string_view>&& sub_arguments)
    {
        m_argument_definer.push_back({name, std::move(sub_arguments)});

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
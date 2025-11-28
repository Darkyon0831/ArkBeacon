#include "ConfigLoader.h"

bool ArkBeacon::Utils::ConfigLoader::LoadConfig(std::string_view file_path)
{
    toml::v3::table config = toml::v3::parse_file(file_path);

    try
    {
        toml::v3::table config = toml::v3::parse_file(file_path);

        auto file_name = file_path.substr(file_path.find_last_of("/") + 1, file_path.length());

        m_configs.insert({std::string(file_name), std::move(config)});
    }
    catch(const toml::v3::parse_error& e)
    {
        std::cerr << "Failed to load config from " << file_path << ": " << e.description() << '\n';
        return false;
    }

    return true;
}

toml::v3::table& ArkBeacon::Utils::ConfigLoader::GetConfig(std::string_view config_name)
{
    auto it = m_configs.find(std::string(config_name));
    if (it != m_configs.end())
    {
        return it->second;
    }

    throw std::runtime_error("Config not found");
}

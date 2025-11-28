#ifndef __ARKBEACON_UTILS_CONFIG_LOADER_H__
#define __ARKBEACON_UTILS_CONFIG_LOADER_H__

#include <string_view>
#include <unordered_map>
#include <toml++/toml.hpp>
#include <iostream>

namespace ArkBeacon {
    namespace Utils {

        class ConfigLoader {
        public:
            ConfigLoader() = default;
            ~ConfigLoader() = default;

            bool LoadConfig(std::string_view file_path);
            toml::v3::table& GetConfig(std::string_view config_name);

            toml::v3::table& operator[](std::string_view config_name) {
                return GetConfig(config_name);
            }

            template <typename T>
            void RegisterValue(std::string_view config_name, const std::string& key, const T& value) {
                m_configs[std::string(config_name)].insert_or_assign(key, value);
            }

        private:
            std::unordered_map<std::string, toml::v3::table> m_configs;
        };
    } // namespace Utils
} // namespace ArkBeacon

#endif // __ARKBEACON_UTILS_CONFIG_LOADER_H__
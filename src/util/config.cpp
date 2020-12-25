#include "pch.h"

#include "util/config.hpp"
#include "util/json.hpp"
#include "util/log.hpp"

namespace util {

Config
Config::load(const std::string& path)
{
    log::Debug("Config", "Loading \"{}\"", path);
    Config config;
    auto cfgFile = LoadJson(path);

    config.address = try_get_default<std::string>(cfgFile, "address", "127.0.0.1");
    config.port = try_get_default<uint16_t>(cfgFile, "port", static_cast<uint16_t>(8080));
    config.updateRate = try_get_default<uint32_t>(cfgFile, "updateRate", static_cast<uint32_t>(60));
    auto logLevel = try_get_default<std::string>(cfgFile, "logLevel", "info");
    return config;
}

} // namespace util
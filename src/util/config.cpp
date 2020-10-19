#include "util/config.hpp"
#include "util/json.hpp"
#include "util/log.hpp"
#include <atomic>

namespace util {

static std::atomic_bool loaded = false;
static auto config = Config{};

const Config&
Config::get()
{
    if (!loaded) {
        log::Debug("Config", "Loading \"config.json\"");
        auto cfgFile = LoadJson("config.json");

        config.address = try_get_default<std::string>(cfgFile, "address", "127.0.0.1");
        config.port = try_get_default<uint16_t>(cfgFile, "port", static_cast<uint16_t>(8080));
        config.threads = try_get_default<uint8_t>(cfgFile, "threads", static_cast<uint8_t>(1));
        config.updateRate = try_get_default<uint32_t>(cfgFile, "updateRate", static_cast<uint32_t>(60));
        auto logLevel = try_get_default<std::string>(cfgFile, "logLevel", "info");

        loaded = true;
    }
    return config;
}

} // namespace util

std::ostream&
operator<<(std::ostream& os, const util::Config& config)
{
    return os << "{"
              << " address: " << config.address << ", port: " << config.port << ", threads: " << config.threads
              << ", updateRate: " << config.updateRate << ", logLevel: " << config.logLevel << " }";
}
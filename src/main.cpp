
#include "net/net.hpp"
#include "util/json.hpp"
#include "util/log.hpp"
#include "util/time.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <string>

struct Config
{
    std::string address;
    uint16_t port;
    uint8_t threads;
};

namespace util {

Config
LoadConfig(const std::string& path = "config.json")
{
    log::Trace("LoadConfig", "Loading \"{}\"", path);
    auto cfgFile = LoadJson(path);

    auto address = try_get_default<std::string>(cfgFile, "address", std::string{ "127.0.0.1" });
    auto port = try_get_default<uint16_t>(cfgFile, "port", static_cast<uint16_t>(8080));
    auto threads = try_get_default<uint8_t>(cfgFile, "threads", static_cast<uint8_t>(1));

    return Config{ address, port, threads };
}

}

// TODO: investigate port failing to bind
// probably need to implement signal handlers
// and come up with a proper way to
// start and stop network threads

int
main()
{
    const auto config = util::LoadConfig();

    net::Context{ config.address, config.port, config.threads };

    auto ticks = 30;
    // time is in seconds
    auto last = util::time::Now();
    while (true) {
        auto now = util::time::Now();
        auto diff = (util::time::Now() - last).count();
        if (diff < 1000)
            continue;

        util::log::Info("main", "Ticks remaining: {}", ticks);
        if (ticks-- == 0) {
            util::log::Info("main", "Shutting down...");
            break;
        }
        last = now;
    }
}

/* int
main()
{
    util::log::SetLevel(util::log::Level::Trace);

    // Check command line arguments.
    const auto config = util::LoadConfig();

    // The io_context is required for all I/O
    asio::io_context ioc;

    // Create and launch a listening port
    auto endpoint = tcp::endpoint{ asio::ip::make_address(config.address), config.port };
    util::log::Info("main", "Server: address: {}, port: {}", endpoint.address().to_string(), endpoint.port());
    net::CreateListener(ioc, endpoint, {})->open();
    ioc.run();

    // Run the I/O service on the requested number of threads
    // std::vector<std::thread> v;
    // v.reserve(config.threads - 1);
    // for (auto i = config.threads - 1; i > 0; --i)
    //    v.emplace_back([&ioc] { ioc.run(); });
    // ioc.run();

    return EXIT_SUCCESS;
} */

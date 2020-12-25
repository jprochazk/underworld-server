#include "pch.h"

#include "game/world.hpp"
#include "net/net.hpp"
#include "server.hpp"

Server::Config
Server::Config::load(const std::string& path)
{
    util::log::Debug("Config", "Loading \"{}\"", path);
    Config config;
    auto cfgFile = util::LoadJson(path);

    config.address = try_get_default<std::string>(cfgFile, "address", "127.0.0.1");
    config.port = try_get_default<uint16_t>(cfgFile, "port", static_cast<uint16_t>(8080));
    config.updateRate = try_get_default<uint32_t>(cfgFile, "updateRate", static_cast<uint32_t>(60));
    auto logLevel = try_get_default<std::string>(cfgFile, "logLevel", "info");
    return config;
}

Server::Server(Config config)
  : config{ config }
  , ioc{}
  , iocThread{}
  , signals{ ioc, SIGINT, SIGTERM } // TODO: have more than 1 world at a time
  , world{ game::CreateWorld() }
  , listener{ net::CreateListener(ioc,
                                  tcp::endpoint{ asio::ip::make_address(config.address), config.port },
                                  world->getHandler()) }
{
    util::log::Info("Server", "Initialized");
    signals.async_wait(util::SignalHandler{});

    iocThread = std::thread([&] {
        listener->open();
        ioc.run();
    });
}

void
Server::run()
{
    util::log::Info("Server", "Running");
    // infinite loop that simply does nothing other than wait for the signal handler to be called
    // TODO: provide another way to terminate the server
    auto interval = 1000.0 / static_cast<double>(config.updateRate);
    auto last = util::time::Now();
    while (!util::SignalHandler::exit) {
        // 1 tick = 1000 ms
        auto now = util::time::Now();
        auto diff = (now - last).count();
        if (diff < interval)
            continue;

        last = now;

        world->update();
    }
    ioc.stop();
}
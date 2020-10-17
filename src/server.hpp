#ifndef SERVER_SERVER_HPP_
#define SERVER_SERVER_HPP_

#include "game/world.hpp"
#include "net/net.hpp"
#include "signal.hpp"
#include "util/util.hpp"

class Server
{
public:
    Server()
      : ioc{ util::Config::get().threads }
      , signals{ ioc, SIGINT, SIGTERM }
      , world{ game::CreateWorld() }
      , context{ ioc, world->getHandler() }
      , updateInterval{ 1000. / static_cast<double>(util::Config::get().updateRate) }
    {
        util::log::Info("Server", "Initialized");
        signals.async_wait(SignalHandler{});
    }

    void
    run()
    {
        util::log::Info("Server", "Running");
        auto last = util::time::Now();
        while (!SignalHandler::exit) {
            // 1 tick = 1000 ms
            auto now = util::time::Now();
            auto diff = (now - last).count();
            if (diff < updateInterval)
                continue;

            last = now;

            update();
        }
    }

private:
    void
    update()
    {
        world->update();
    }

    asio::io_context ioc;
    asio::signal_set signals;
    std::shared_ptr<game::World> world;
    net::Context context;
    double updateInterval;
}; // class Server

#endif // SERVER_SERVER_HPP_
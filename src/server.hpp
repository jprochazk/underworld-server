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
      , signals{ ioc, SIGINT, SIGTERM } // TODO: have more than 1 world at a time
      , worldManager{ game::CreateWorldManager(1u /* util::Config::get().threads */) }
      , context{ ioc, worldManager }
    {
        util::log::Info("Server", "Initialized");
        signals.async_wait(SignalHandler{});
    }

    void
    run()
    {
        util::log::Info("Server", "Running");
        worldManager->start();
        // infinite loop that simply does nothing other than wait for the signal handler to be called
        // TODO: provide another way to terminate the server
        while (!SignalHandler::exit) {
        }
        worldManager->stop();
    }

private:
    asio::io_context ioc;
    asio::signal_set signals;
    std::shared_ptr<game::WorldManager> worldManager;
    net::Context context;
}; // class Server

#endif // SERVER_SERVER_HPP_
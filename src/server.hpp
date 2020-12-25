#include "pch.h"

#ifndef SERVER_SERVER_HPP_
#define SERVER_SERVER_HPP_

#include "util/util.hpp"

namespace game {
class World;
}
namespace net {
class Listener;
}

class Server
{
public:
    struct Config
    {
        std::string address;
        uint16_t port;
        uint16_t updateRate;
        int logLevel;

        static Config load(const std::string& path);
    };

    Server(Config config);
    void run();

private:
    Config config;
    asio::io_context ioc;
    util::ScopedThread iocThread;
    asio::signal_set signals;
    std::shared_ptr<game::World> world;
    std::shared_ptr<net::Listener> listener;
}; // class Server

#endif // SERVER_SERVER_HPP_
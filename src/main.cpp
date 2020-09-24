
#include "net/net.hpp"
#include "util/json.hpp"
#include "util/log.hpp"
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

int
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
}

/* namespace net {

class Context
{
    asio::io_context ioc_;
    tcp::endpoint endpoint_;
    std::shared_ptr<net::Handler> handler_;
    std::shared_ptr<net::Listener> listener_;
    std::vector<std::thread> threads_;

public:
    Context(Config config, std::shared_ptr<net::Handler> handler = {})
      : ioc_{ config.threads }
      , endpoint_{ tcp::endpoint{ asio::ip::make_address(config.address), config.port } }
      , handler_{ handler }
      , listener_{ nullptr }
      , threads_{ static_cast<size_t>(config.threads) }
    {}

    void start()
    {
        // start the socket listener
        listener_ = net::CreateListener(ioc_, endpoint_, handler_);
        listener_->open();

        // then start the I/O event loop
        threads_.clear();
        for (size_t i = 0; i < threads_.size(); ++i)
            threads_.emplace_back([& ioc_ = ioc_] { ioc_.run(); });
    }

    void stop() { ioc_.stop(); }
}; // class Context

} // namespace net */

/* int
main()
{
    const auto config = util::LoadConfig();

    auto context = net::Context{ config };
    context.start();

    auto ticks = 10;
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
            context.stop();
            break;
        }
        last = now;
    }
} */

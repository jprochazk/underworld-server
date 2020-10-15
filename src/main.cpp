
#include "game/world.hpp"
#include "net/listener.hpp"
#include "net/net.hpp"
#include "net/packet.hpp"
#include "util/json.hpp"
#include "util/log.hpp"
#include "util/thread.hpp"
#include "util/time.hpp"
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <moodycamel/concurrentqueue.h>
#include <optional>
#include <string>

struct Config
{
    std::string address;
    uint16_t port;
    uint8_t threads;
    uint32_t updateRate;
    util::log::Level logLevel;
};
std::ostream&
operator<<(std::ostream& out, const Config& cfg)
{
    return out << "address: " << cfg.address << ", "
               << "port: " << cfg.port << ", "
               << "threads: " << cfg.threads << ", "
               << "updateRate: " << cfg.updateRate << ", "
               << "logLevel: " << util::log::ToString(cfg.logLevel) << std::endl;
}

namespace util {

Config
LoadConfig(const std::string& path = "config.json")
{
    log::Debug("LoadConfig", "Loading \"{}\"", path);
    auto cfgFile = LoadJson(path);

    Config cfg{};

    cfg.address = try_get_default<std::string>(cfgFile, "address", "127.0.0.1");
    cfg.port = try_get_default<uint16_t>(cfgFile, "port", static_cast<uint16_t>(8080));
    cfg.threads = try_get_default<uint8_t>(cfgFile, "threads", static_cast<uint8_t>(1));
    cfg.updateRate = try_get_default<uint32_t>(cfgFile, "updateRate", static_cast<uint32_t>(60));
    auto logLevelStr = try_get_default<std::string>(cfgFile, "logLevel", "info");
    const std::unordered_map<std::string, util::log::Level> logLevels = {
        { "trace", util::log::Level::Trace }, { "debug", util::log::Level::Debug },
        { "info", util::log::Level::Info },   { "warn", util::log::Level::Warn },
        { "error", util::log::Level::Error }, { "fatal", util::log::Level::Critical }
    };
    cfg.logLevel = logLevels.at(logLevelStr);

    return cfg;
}

}

std::map<size_t, std::string> signals = {
    { 1, "SIGHUP" },     { 2, "SIGINT" },   { 3, "SIGQUIT" },   { 4, "SIGILL" },   { 5, "SIGTRAP" },
    { 6, "SIGABRT" },    { 7, "SIGBUS" },   { 8, "SIGFPE" },    { 9, "SIGKILL" },  { 10, "SIGUSR1" },
    { 11, "SIGSEGV" },   { 12, "SIGUSR2" }, { 13, "SIGPIPE" },  { 14, "SIGALRM" }, { 15, "SIGTERM" },
    { 16, "SIGSTKFLT" }, { 17, "SIGCLD" },  { 18, "SIGCONT" },  { 19, "SIGSTOP" }, { 20, "SIGTSTP" },
    { 21, "SIGTTIN" },   { 22, "SIGTTOU" }, { 23, "SIGURG" },   { 24, "SIGXCPU" }, { 25, "SIGXFSZ" },
    { 26, "SIGVTALRM" }, { 27, "SIGPROF" }, { 28, "SIGWINCH" }, { 29, "SIGIO" },   { 30, "SIGPWR" },
    { 31, "SIGSYS" },
};

std::atomic<bool> ShouldExit = false;
void
HandleSignal(const boost::system::error_code& ec, int signalNum)
{
    if (!ec) {
        util::log::Info("HandleSignal", "Signal {}", signals[signalNum]);
        ShouldExit = true;
    }
}

namespace net {

class Context
{
    asio::io_context& ioc_;
    std::vector<util::ScopedThread> threads_;
    std::shared_ptr<Listener> listener_;
    std::shared_ptr<Handler> handler_;

    std::string address_;
    uint16_t port_;
    uint8_t numThreads_;

public:
    Context(asio::io_context& ioc,
            std::string address,
            uint16_t port,
            uint8_t numThreads,
            std::shared_ptr<Handler> handler)
      : ioc_{ ioc }
      , threads_{ numThreads }
      , listener_{ nullptr }
      , handler_{ handler }
      , address_{ address }
      , port_{ port }
      , numThreads_{ numThreads }
    {
        listener_ = CreateListener(ioc_, tcp::endpoint{ asio::ip::make_address(address_), port_ }, handler_);
        listener_->open();

        threads_.clear();
        threads_.reserve(numThreads_);
        for (size_t i = 0; i < numThreads_; i++) {
            util::log::Info("Network", "Starting thread #{}", i + 1);
            threads_.emplace_back([&] { ioc_.run(); });
        }
    }

    ~Context() { ioc_.stop(); }
};

} // namespace net

int
main()
{
    util::log::SetLevel(util::log::Level::Info);
    const auto config = util::LoadConfig();

    asio::io_context ioc;
    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(HandleSignal);

    auto world = game::CreateWorld();
    net::Context context{ ioc, config.address, config.port, config.threads, world->getHandler() };

    util::log::Info("main", "Server is ready, starting main loop...");

    // Server will live for 30 seconds and then shutdown
    // During this time, sockets may connect and send messages.
    const auto updateInterval = 1000. / static_cast<double>(config.updateRate);
    auto last = util::time::Now();
    while (!ShouldExit) {
        // 1 tick = 1000 ms
        auto now = util::time::Now();
        auto diff = (now - last).count();
        if (diff < updateInterval)
            continue;

        last = now;

        world->update();
    }
}

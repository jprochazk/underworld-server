
#include "game/session.hpp"
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
    util::log::SetLevel(util::log::Level::Trace);
    const auto config = util::LoadConfig();

    asio::io_context ioc;
    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait(HandleSignal);

    auto sessionMgr = game::CreateSessionManager();
    net::Context context{ ioc, config.address, config.port, config.threads, sessionMgr };

    util::log::Info("main", "Server is ready, starting main loop...");

    // Server will live for 30 seconds and then shutdown
    // During this time, sockets may connect and send messages.
    auto last = util::time::Now();
    while (!ShouldExit) {
        // 1 tick = 1000 ms
        auto now = util::time::Now();
        auto diff = (now - last).count();
        if (diff < 1000)
            continue;

        last = now;

        sessionMgr->update();
    }
}


// clang-format off
#include "util/log.hpp"
#include "util/config.hpp"
#include "server.hpp"
// clang-format on

/*
TODO: re-structure the server to be (almost) entirely single-threaded
    -> network is STILL on a separate thread, because
       we still need to seperate the IOC thread from the main loop one.
    -> remove world manager, there is only a single world
    -> remove router concept
    -> remove context - server only needs to hold the listener
    -> script stuff no longer needs to worry about thread safety!!! WOOO
TODO: make the script context follow the separate interface/impl structure
TODO: don't store any global state - because there is only a single context per server
TODO: instead of script::Context::execute(), do script::Context::get() instead
      to allow multiple execution of the same script
TODO: precompiled header
TODO: DB infrastructure
TODO: containerize the server (docker)
NOTE: in case horizontal scaling is needed, use kubernetes
TODO: static analysis
TODO: automated tests
TODO: CI/CD
*/

int
main()
{
    util::log::SetLevel(static_cast<util::log::Level>(util::Config::get().logLevel));
    try {
        Server server;
        server.run();
    } catch (std::exception& ex) {
        util::log::Critical("Server encountered fatal error: {}", ex.what());
    }

    return 0;
}

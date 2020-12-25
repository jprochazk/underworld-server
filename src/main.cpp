#include "pch.h"

// clang-format off
#include "util/log.hpp"
#include "util/config.hpp"
#include "server.hpp"
// clang-format on

/*
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
    Server::Config config = Server::Config::load("config.json");
    util::log::SetLevel(static_cast<util::log::Level>(config.logLevel));

    try {
        Server server(config);
        server.run();
    } catch (std::exception& ex) {
        util::log::Critical("Server encountered fatal error: {}", ex.what());
    }

    return 0;
}

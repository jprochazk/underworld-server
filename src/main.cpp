
// clang-format off
#include "util/log.hpp"
#include "server.hpp"
// clang-format on

// TODO: LUA infrastructure
// TODO: DB infrastructure

int
main()
{
    try {
        Server server;
        server.run();
    } catch (std::exception& ex) {
        util::log::Critical("Server encountered fatal error: {}", ex.what());
    }

    return 0;
}

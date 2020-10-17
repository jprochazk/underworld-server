
// clang-format off
#include "util/log.hpp"
#include "server.hpp"
// clang-format on

int
main()
{
    util::log::SetLevel(static_cast<util::log::Level>(util::Config::get().logLevel));

    Server server;
    server.run();
}

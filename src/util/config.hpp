#ifndef SERVER_UTIL_CONFIG_HPP_
#define SERVER_UTIL_CONFIG_HPP_

#include "util/log.hpp"
#include <ostream>
#include <string>

namespace util {

struct Config
{
    std::string address;
    uint16_t port;
    uint8_t threads;
    uint16_t updateRate;
    int logLevel;

    // TODO: scrap lazy initialization, and use a static ::init() instead.
    // lazily initialized
    static const Config& get();
};

} // namespace util

std::ostream& operator<<(std::ostream& os, const util::Config& config);

#endif // SERVER_UTIL_CONFIG_HPP_
#include "pch.h"

#ifndef SERVER_UTIL_CONFIG_HPP_
#define SERVER_UTIL_CONFIG_HPP_

#include "util/log.hpp"

namespace util {

struct Config
{
    std::string address;
    uint16_t port;
    uint16_t updateRate;
    int logLevel;

    static Config load(const std::string& path);
};

} // namespace util

#endif // SERVER_UTIL_CONFIG_HPP_
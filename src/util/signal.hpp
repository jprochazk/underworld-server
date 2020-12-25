#include "pch.h"

#ifndef SERVER_UTIL_SIGNAL_HPP_
#define SERVER_UTIL_SIGNAL_HPP_

namespace util {

struct SignalHandler
{
    static std::atomic_bool exit;

    void operator()(const boost::system::error_code&, int);
};

} // namespace util

#endif // SERVER_UTIL_SIGNAL_HPP_
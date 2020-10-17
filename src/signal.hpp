#ifndef SERVER_SIGNAL_HPP_
#define SERVER_SIGNAL_HPP_

#include <atomic>

namespace boost::system {
class error_code;
}

struct SignalHandler
{
    static std::atomic_bool exit;

    void operator()(const boost::system::error_code&, int);
};

#endif // SERVER_SIGNAL_HPP_
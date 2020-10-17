#include "signal.hpp"
#include "net/boost.hpp"

std::atomic_bool SignalHandler::exit = false;

void
SignalHandler::operator()(const boost::system::error_code& ec, int /* signal */)
{
    if (!ec) {
        exit = true;
    }
}
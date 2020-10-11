#ifndef SERVER_NET_LISTENER_HPP_
#define SERVER_NET_LISTENER_HPP_

#include "net/boost.hpp"
#include <memory>

namespace net {

class Handler;

class Listener
{
public:
    virtual ~Listener() = default;

    virtual void open() = 0;
}; // class Listener

std::shared_ptr<Listener> CreateListener(asio::io_context& ioc,
                                         tcp::endpoint endpoint,
                                         std::shared_ptr<Handler> handler);

} // namespace net

#endif // SERVER_NET_LISTENER_HPP_
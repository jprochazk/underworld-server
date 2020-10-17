#ifndef SERVER_NET_CONTEXT_HPP_
#define SERVER_NET_CONTEXT_HPP_

#include "net/boost.hpp"
#include "net/handler.hpp"
#include "net/listener.hpp"
#include "util/log.hpp"
#include "util/thread.hpp"
#include <memory>
#include <vector>

namespace net {

class Context
{
public:
    Context(asio::io_context& ioc, std::shared_ptr<Router> router);
    ~Context();

private:
    asio::io_context& ioc_;
    std::vector<util::ScopedThread> threads_;
    std::shared_ptr<Listener> listener_;
    std::shared_ptr<Router> router_;

    std::string address_;
    uint16_t port_;
    uint8_t numThreads_;
}; // class Context

} // namespace net

#endif // SERVER_NET_CONTEXT_HPP_
#ifndef SERVER_NET_NET_HPP
#define SERVER_NET_NET_HPP

#include "handler.hpp"
#include "listener.hpp"
#include "socket.hpp"
#include "util/log.hpp"

namespace net {

class Context
{
    asio::io_context ioc_;
    tcp::endpoint endpoint_;
    std::shared_ptr<net::Handler> handler_;
    std::shared_ptr<net::Listener> listener_;
    std::vector<std::thread> threads_;

public:
    Context(const std::string& address, uint16_t port, uint8_t numThreads, std::shared_ptr<net::Handler> handler = {})
      : ioc_{ numThreads }
      , endpoint_{ tcp::endpoint{ asio::ip::make_address(address), port } }
      , handler_{ handler }
      , listener_{ nullptr }
      , threads_{}
    {
        util::log::Info("Context", "Starting network context...");
        // start the socket listener
        listener_ = net::CreateListener(ioc_, endpoint_, handler_);
        listener_->open();

        // then start the I/O event loop
        threads_.reserve(numThreads);
        for (size_t i = 0; i < numThreads; ++i) {
            util::log::Info("Context", "Starting network thread #{}", i + 1);
            threads_.emplace_back([& ioc_ = this->ioc_] { ioc_.run(); });
        }
    }
    ~Context()
    {
        ioc_.stop();
        for (auto& thread : threads_) {
            thread.join();
        }
    }
}; // class Context

} // namespace net

#endif // SERVER_NET_NET_HPP
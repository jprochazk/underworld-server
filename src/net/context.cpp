#include "net/context.hpp"
#include "util/config.hpp"

namespace net {

Context::Context(asio::io_context& ioc, std::shared_ptr<Handler> handler)
  : ioc_{ ioc }
  , threads_{ util::Config::get().threads }
  , listener_{ nullptr }
  , handler_{ handler }
  , address_{ util::Config::get().address }
  , port_{ util::Config::get().port }
  , numThreads_{ util::Config::get().threads }
{
    listener_ = CreateListener(ioc_, tcp::endpoint{ asio::ip::make_address(address_), port_ }, handler_);
    listener_->open();

    threads_.clear();
    threads_.reserve(numThreads_);
    for (size_t i = 0; i < numThreads_; i++) {
        util::log::Info("Network", "Starting thread #{}", i + 1);
        threads_.emplace_back([&] { ioc_.run(); });
    }
}

Context::~Context()
{
    ioc_.stop();
}

} // namespace net
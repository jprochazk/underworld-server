#include "listener.hpp"
#include "net/handler.hpp"
#include "net/socket.hpp"
#include "util/log.hpp"
#include <memory>

namespace net {
class Handler;

class ListenerImpl final
  : public Listener
  , public std::enable_shared_from_this<ListenerImpl>
{
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<Handler> handler_;
    uint32_t idseq_;

public:
    ListenerImpl(asio::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<Handler> handler)
      : ioc_{ ioc }
      , acceptor_{ ioc }
      , handler_{ handler }
      , idseq_{ 0 }
    {
        // if (!handler_) {
        //    handler_ = std::make_shared<default_socket_handler>();
        //}

        beast::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) {
            fail("Failed to open endpoint", ec);
            return;
        }

        acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
        if (ec) {
            fail("Failed to set option \"reuse_address\"", ec);
            return;
        }

        acceptor_.bind(endpoint, ec);
        if (ec) {
            fail("Failed to bind to endpoint", ec);
            return;
        }

        acceptor_.listen(asio::socket_base::max_listen_connections, ec);
        if (ec) {
            fail("Failed to start listening on endpoint", ec);
            return;
        }

        util::log::Trace("ListenerImpl", "Listener is initialized");
    }

    virtual void open() override
    {
        util::log::Trace("ListenerImpl", "Opening listener port");
        // start the accept loop
        acceptor_.async_accept(asio::make_strand(ioc_),
                               beast::bind_front_handler(&ListenerImpl::onAccept, shared_from_this()));
    }

private:
    void fail(std::string_view what, beast::error_code ec)
    {
        // if (ec == asio::error::operation_aborted || ec == asio::error::connection_aborted ||
        //    ec == beast::websocket::error::closed)
        //    return;

        util::log::Error("ListenerImpl", "{}: {}", what, ec.message());
    }

    void onAccept(beast::error_code ec, tcp::socket socket)
    {
        util::log::Trace("ListenerImpl::onAccept", "Got socket");

        if (ec) {
            return fail("onAccept", ec);
        }

        CreateSocket(idseq_++, std::move(socket), handler_)->open();

        acceptor_.async_accept(asio::make_strand(ioc_),
                               beast::bind_front_handler(&ListenerImpl::onAccept, shared_from_this()));
    }
}; // class ListenerImpl

class DefaultHandlerImpl : public Handler
{
public:
    virtual void onOpen(uint32_t id, std::weak_ptr<net::Socket> /* socket */) override
    {
        util::log::Info("DefaultHandlerImpl", "Socket {{ ID = {} }} -> open", id);
    }
    virtual void onClose(uint32_t id) override
    {
        util::log::Info("DefaultHandlerImpl", "Socket {{ ID = {} }} -> close", id);
    }
    virtual void onMessage(uint32_t id, std::vector<uint8_t>&& data) override
    {
        util::log::Info("DefaultHandlerImpl", "Socket {{ ID = {} }} -> message size {}", id, data.size());
    }
    // Sockets that encounter an error aren't closed.
    virtual void onError(uint32_t id, std::string_view what, beast::error_code error) override
    {
        util::log::Info("DefaultHandlerImpl", "Socket {{ ID = {} }} -> error: {}, {}", id, what, error.message());
    }
};

std::shared_ptr<Listener>
CreateListener(asio::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<Handler> handler)
{
    return std::make_shared<ListenerImpl>(
      ioc, endpoint, static_cast<bool>(handler) ? handler : std::make_shared<DefaultHandlerImpl>());
}

} // namespace net
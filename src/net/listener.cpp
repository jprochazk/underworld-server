#include "listener.hpp"
#include "boost/beast/core/bind_handler.hpp"
#include "net/handler.hpp"
#include "net/socket.hpp"
#include "util/log.hpp"
#include <chrono>
#include <memory>

namespace net {

class Handler;

class Authenticator : public std::enable_shared_from_this<Authenticator>
{
public:
    Authenticator(uint32_t id, tcp::socket&& socket, std::shared_ptr<net::Handler> handler)
      : id_{ id }
      , stream_{ std::move(socket) }
      , buffer_{}
      , parser_{}
      , handler_{ handler }
    {}

    void
    run()
    {
        parser_.body_limit(2048);
        stream_.expires_after(std::chrono::seconds(10));
        http::async_read(
          stream_, buffer_, parser_.get(), beast::bind_front_handler(&Authenticator::onRead, shared_from_this()));
    }

private:
    void
    error(std::string_view what, beast::error_code ec)
    {
        // if (ec == asio::error::operation_aborted || ec == asio::error::connection_aborted ||
        //    ec == beast::websocket::error::closed)
        //    return;

        util::log::Error("Authenticator", "{}: {}", what, ec.message());
    }

    void
    write(http::status code)
    {
        http::response<http::string_body> res{ code, 11 };
        http::write(stream_, res);
    }

    void
    onRead(beast::error_code ec, std::size_t)
    {
        if (ec == http::error::end_of_stream) {
            stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        }

        if (ec)
            return error("onRead", ec);

        auto req = parser_.get();
        if (beast::websocket::is_upgrade(req)) {
            // TODO: check for actual auth token
            auto protocol = req[http::field::sec_websocket_protocol];
            std::string token{ protocol.begin(), protocol.end() };
            if (token == "test") {
                net::CreateSocket(id_, stream_.release_socket(), handler_)->open(parser_.release(), token);
                return;
            } else {
                return write(http::status::unauthorized);
            }
        }

        write(http::status::bad_request);
    }

    uint32_t id_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    http::request_parser<http::string_body> parser_;
    std::shared_ptr<net::Handler> handler_;
}; // class Authenticator

class ListenerImpl final
  : public Listener
  , public std::enable_shared_from_this<ListenerImpl>
{
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<Router> router_;
    uint32_t idseq_;

public:
    ListenerImpl(asio::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<Router> router)
      : ioc_{ ioc }
      , acceptor_{ ioc }
      , router_{ router }
      , idseq_{ 0 }
    {

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

        // Disable Nagle's algorithm (packets are sent immediately, instead of waiting for a full buffer)
        acceptor_.set_option(asio::ip::tcp::no_delay{ true });

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

        util::log::Debug("ListenerImpl", "Listener is initialized");
    }

    virtual void
    open() override
    {
        util::log::Debug("ListenerImpl", "Opening listener port");
        // start the accept loop
        acceptor_.async_accept(asio::make_strand(ioc_),
                               beast::bind_front_handler(&ListenerImpl::onAccept, shared_from_this()));
    }

private:
    void
    fail(std::string_view what, beast::error_code ec)
    {
        // if (ec == asio::error::operation_aborted || ec == asio::error::connection_aborted ||
        //    ec == beast::websocket::error::closed)
        //    return;

        util::log::Error("ListenerImpl", "{}: {}", what, ec.message());
    }

    void
    onAccept(beast::error_code ec, tcp::socket socket)
    {
        util::log::Debug("ListenerImpl::onAccept", "Got socket");

        if (ec) {
            return fail("onAccept", ec);
        }

        std::make_shared<Authenticator>(idseq_++, std::move(socket), router_->select())->run();

        acceptor_.async_accept(asio::make_strand(ioc_),
                               beast::bind_front_handler(&ListenerImpl::onAccept, shared_from_this()));
    }
}; // class ListenerImpl

std::shared_ptr<Listener>
CreateListener(asio::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<Router> router)
{
    return std::make_shared<ListenerImpl>(ioc, endpoint, router);
}

} // namespace net
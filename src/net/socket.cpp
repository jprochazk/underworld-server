#include "socket.hpp"
#include "net/handler.hpp"
#include "util/log.hpp"
#include <cstddef>
#include <memory>
#include <stdexcept>

namespace net {

constexpr size_t MAX_MESSAGE_SIZE = 256;

class SocketImpl final
  : public Socket
  , public std::enable_shared_from_this<SocketImpl>
{
    uint32_t id_;
    beast::flat_buffer readBuffer_;
    beast::websocket::stream<beast::tcp_stream> stream_;
    std::vector<std::vector<uint8_t>> writeBuffer_;

    std::atomic<bool> isWriting_;
    std::atomic<bool> open_;
    std::shared_ptr<Handler> handler_;

public:
    SocketImpl(uint32_t id, tcp::socket&& socket, std::shared_ptr<Handler> handler)
      : id_{ id }
      , readBuffer_{}
      , stream_{ std::move(socket) }
      , writeBuffer_{}
      , isWriting_{ false }
      , open_{ false }
      , handler_{ handler }
    {}

    virtual ~SocketImpl() override
    {
        // metrics::disconnection();
        handler_->onClose(id_);
    }

    virtual void
    open() override
    {
        if (isOpen())
            return;

        stream_.set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::server));

        stream_.set_option(beast::websocket::stream_base::decorator([](beast::websocket::response_type& res) {
            res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server");
        }));

        // any message above MAX_MESSAGE_SIZE bytes will be discarded
        stream_.read_message_max(MAX_MESSAGE_SIZE);

        stream_.binary(true);

        open_ = true;

        stream_.async_accept(beast::bind_front_handler(&SocketImpl::onAccept, shared_from_this()));
    }

    virtual void
    close() override
    {
        if (isOpen())
            return;

        util::log::Trace("SocketImpl::close", "Closing socket ID {}", id_);
        stream_.async_close(beast::websocket::close_code::normal,
                            beast::bind_front_handler(&SocketImpl::onClose, shared_from_this()));
    }

    virtual void
    send(std::vector<uint8_t> data) override
    {
        if (!isOpen())
            return;

        util::log::Trace("SocketImpl::send", "Sending {} bytes to socket ID {}", data.size(), id_);
        asio::post(stream_.get_executor(),
                   beast::bind_front_handler(&SocketImpl::onSend, shared_from_this(), std::move(data)));
    }

    virtual uint32_t
    getId() override
    {
        return id_;
    }
    virtual bool
    isOpen() override
    {
        return open_;
    }

private:
    void
    onAccept(beast::error_code ec)
    {
        if (ec)
            return handler_->onError(id_, "Socket::onAccept", ec);

        // metrics::connection();
        handler_->onOpen(id_, weak_from_this());

        stream_.async_read(readBuffer_, beast::bind_front_handler(&SocketImpl::onRead, shared_from_this()));
    }

    void
    onClose(beast::error_code ec)
    {
        open_ = false;
        if (ec)
            return handler_->onError(id_, "Socket::onClose", ec);
    }

    void
    onRead(beast::error_code ec, std::size_t readBytes)
    {
        if (ec)
            return handler_->onError(id_, "Socket::onRead", ec);

        // metrics::read(read_bytes);

        // std::vector<uint8_t> buf(readBytes);
        // boost::asio::buffer_copy(boost::asio::buffer(buf.data(), readBytes), readBuffer_.data());
        // handler_->onMessage(id_, std::move(buf));

        auto buffer = readBuffer_.data();
        handler_->onMessage(id_, static_cast<uint8_t*>(buffer.data()), buffer.size());
        readBuffer_.consume(readBytes);

        stream_.async_read(readBuffer_, beast::bind_front_handler(&SocketImpl::onRead, shared_from_this()));
    }

    void
    onSend(std::vector<uint8_t>&& data)
    {
        writeBuffer_.push_back(std::move(data));

        if (isWriting_)
            return;
        isWriting_ = true;

        stream_.async_write(asio::buffer(writeBuffer_.front()),
                            beast::bind_front_handler(&SocketImpl::onWrite, shared_from_this()));
    }

    void
    onWrite(beast::error_code ec, std::size_t /* writtenBytes */)
    {
        if (ec)
            return handler_->onError(id_, "Socket::onWrite", ec);

        // metrics::written(writtenBytes);

        writeBuffer_.erase(writeBuffer_.begin());

        if (writeBuffer_.empty()) {
            isWriting_ = false;
            return;
        }

        stream_.async_write(asio::buffer(writeBuffer_.front()),
                            beast::bind_front_handler(&SocketImpl::onWrite, shared_from_this()));
    }

}; // class SocketImpl

std::shared_ptr<Socket>
CreateSocket(uint32_t id, tcp::socket&& socket, std::shared_ptr<Handler> handler)
{
    util::log::Trace("CreateListener", "New socket ID {}", id);
    return std::make_shared<SocketImpl>(id, std::move(socket), handler);
}

} // namespace net
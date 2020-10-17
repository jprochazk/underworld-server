#ifndef SERVER_NET_HANDLER_HPP_
#define SERVER_NET_HANDLER_HPP_

#include "net/boost.hpp"
#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

namespace net {

class Socket;

class Handler
{
public:
    virtual ~Handler() = default;
    virtual uint16_t id() const = 0;
    virtual void onOpen(uint32_t id, std::weak_ptr<Socket> socket) = 0;
    virtual void onClose(uint32_t id) = 0;
    /// virtual void onMessage(uint32_t id, std::vector<uint8_t>&& data) = 0;
    virtual void onMessage(uint32_t id, uint8_t* data, size_t size) = 0;
    // Sockets that encounter an error aren't closed.
    virtual void onError(uint32_t id, std::string_view what, beast::error_code error) = 0;
}; // class Handler

class Router
{
public:
    virtual ~Router() = default;
    virtual std::shared_ptr<Handler> select() = 0;
    virtual std::shared_ptr<Handler> select(uint16_t) = 0;
}; // class LoadBalancer

} // namespace net

#endif // SERVER_NET_HANDLER_HPP_

#ifndef SERVER_NET_SOCKET_HPP_
#define SERVER_NET_SOCKET_HPP_

#include "net/boost.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace net {

class Handler;

class Socket
{
public:
    virtual ~Socket() = default;
    virtual void open(http::request<http::string_body> req, std::string token) = 0;
    virtual void close() = 0;
    // TODO: make an overload for sending shared_ptr<vector<uint8_t>> data
    virtual void send(std::vector<uint8_t> data) = 0;

    virtual uint32_t getId() = 0;
    virtual bool isOpen() = 0;
}; // class Socket

std::shared_ptr<Socket> CreateSocket(uint32_t id, tcp::socket&& socket, std::shared_ptr<Handler> handler);

} // namespace net

#endif // SERVER_NET_SOCKET_HPP_
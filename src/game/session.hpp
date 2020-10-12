#ifndef SERVER_GAME_SESSION_HPP_
#define SERVER_GAME_SESSION_HPP_

#include "net/handler.hpp"
#include "util/log.hpp"
#include <cstdint>
#include <memory>

namespace game {

class Session
{
public:
    virtual ~Session() = default;
    virtual bool send(const std::vector<uint8_t>& data) = 0;
    virtual bool isOpen() const = 0;
    virtual uint32_t getId() const = 0;
}; // class Session

class SessionManager : public net::Handler
{
public:
    virtual ~SessionManager() = default;
    virtual void onOpen(uint32_t id, std::weak_ptr<net::Socket> socket) override = 0;
    virtual void onClose(uint32_t id) override = 0;
    virtual void onMessage(uint32_t id, uint8_t* data, size_t size) override = 0;
    virtual void onError(uint32_t id, std::string_view what, beast::error_code error) override = 0;

    virtual void update() = 0;
    virtual bool has(uint32_t id) const = 0;
    virtual bool isOpen(uint32_t id) const = 0;
    virtual Session& find(uint32_t id) = 0;
    virtual const Session& find(uint32_t id) const = 0;
}; // class SessionManager

std::shared_ptr<SessionManager> CreateSessionManager();

} // namespace game

#endif // SERVER_GAME_SESSION_HPP_
#include "pch.h"

#include "game/player.hpp"
#include "net/socket.hpp"

namespace game {

struct Session
{
    std::weak_ptr<net::Socket> socket;

    void
    send(std::vector<uint8_t>&& data)
    {
        if (auto conn = socket.lock()) {
            conn->send(std::move(data));
        }
    }
};

Player
Player::create(entt::registry& registry, std::weak_ptr<net::Socket> socket)
{
    auto e = registry.create();
    registry.emplace<Session>(e, Session{ socket });
    return Player{ e, registry };
}

void
Player::destroy()
{
    registry.destroy(entity);
}

bool
Player::valid()
{
    return registry.has<Session>(entity);
}

std::shared_ptr<net::Socket>
Player::getSocket()
{
    return registry.get<Session>(entity).socket.lock();
}

} // namespace game
#ifndef SERVER_GAME_WORLD_HPP_
#define SERVER_GAME_WORLD_HPP_

#include "net/handler.hpp"

// TODO: implement WorldManager

namespace game {

class World
{
public:
    virtual ~World() = default;
    virtual void update() = 0;
    virtual std::shared_ptr<net::Handler> getHandler() = 0;
    virtual size_t size() const = 0;
}; // class World

std::shared_ptr<World> CreateWorld();

} // namespace game

#endif // SERVER_GAME_WORLD_HPP_
#ifndef SERVER_GAME_WORLD_HPP_
#define SERVER_GAME_WORLD_HPP_

#include "net/handler.hpp"

// TODO: documentation

namespace game {

class World
{
public:
    virtual ~World() = default;
    virtual uint16_t id() const = 0;
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual void update() = 0;
    virtual std::shared_ptr<net::Handler> getHandler() = 0;
    virtual size_t size() const = 0;
}; // class World

std::shared_ptr<World> CreateWorld(uint16_t);

class WorldManager : public net::Router
{
public:
    virtual ~WorldManager() = default;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual std::shared_ptr<World> get(uint16_t) = 0;
    virtual std::shared_ptr<net::Handler> select() = 0;
    virtual std::shared_ptr<net::Handler> select(uint16_t) = 0;
}; // class WorldManager

std::shared_ptr<WorldManager> CreateWorldManager(size_t);

} // namespace game

#endif // SERVER_GAME_WORLD_HPP_
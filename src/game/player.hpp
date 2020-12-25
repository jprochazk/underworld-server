#ifndef SERVER_GAME_PLAYER_HPP_
#define SERVER_GAME_PLAYER_HPP_

#include <cstdint>
#include <entt/entt.hpp>
#include <memory>
#include <vector>

namespace net {
class Socket;
}

namespace game {

// Thin wrapper over entt registry & entity
//
// Constructing/destructing a Player is not the same as creating/destroying one in the entt::registry.
// This is just a wrapper to an already existing player in the registry.
struct Player
{
    entt::entity entity;
    entt::registry& registry;

    // TODO: name this something other than create, it's a bit misleading
    // maybe "build"?

    // Create a Player from its parts
    //
    // This creates a new entity, and emplaces all the necessary components into it
    static Player create(entt::registry& registry, std::weak_ptr<net::Socket> socket);

    // TODO: static

    // Destroy a Player
    //
    // This destroys the entity and all its components
    void destroy();

    // Check if a Player is valid
    //
    // Analogous to checking if the entity has all components which make up a Player
    bool valid();

    // Does not check if Player is valid!
    std::shared_ptr<net::Socket> getSocket();
};

} // namespace game

#endif // SERVER_GAME_PLAYER_HPP_
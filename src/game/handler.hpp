#ifndef SERVER_GAME_HANDLER_HPP_
#define SERVER_GAME_HANDLER_HPP_

#include <cstddef>
#include <cstdint>

/*
socket onAccept -> choose world by population/ID (abstracted through WorldManager)
        -> connect (returns World as net::Handler)
        -> handler = world
        -> start read loop

World : net::Handler
        onOpen(ref<socket>) -> enqueue(Player::Connect{ socket->id(), ref<socket> })
        onClose(ref<socket>) -> enqueue(Player::Disconnect{ socket->id(), ref<socke> })
        onMessage(ref<socket>, data) -> enqueue(Player::Message{ socket->id(), data })
        onError(ref<socket>, error) -> game::Handle(Player::Error{ socket->id(), error })
*/

/// To add new Opcodes + Handlers:
/// 1. Add the opcode into the Opcode enum
/// 2. Add it as a case into the switch statement @handler.cpp
/// 3. Implement the handler for it

namespace net {

class Packet;

} // namespace net

namespace game {

struct Player;

// Can be used to provide extra things to the opcode handlers
// without having to modify the signature
struct Context
{
    Player& player;
};

enum Opcode : uint16_t
{
    Test = 0,
    Jump = 1
};

// TODO: change this to be something more easily testable
// currently, for testing purposes, there is a literal "test" packet, which does nothing other than
// give us the ability to verify that this was called
// similar to net::Handler?
void Handle(game::Context& context, game::Opcode opcode, net::Packet& packet);

} // namespace game

#endif // SERVER_GAME_HANDLER_HPP_
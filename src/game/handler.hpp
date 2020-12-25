#include "pch.h"

#ifndef SERVER_GAME_HANDLER_HPP_
#define SERVER_GAME_HANDLER_HPP_

/// To add new Opcodes + Handlers:
/// 1. Add the opcode into the Opcode enum
/// 2. Add it as a case into the switch statement @handler.cpp
/// 3. Implement the handler for it

namespace net {

class Packet;

} // namespace net

namespace game {

struct Player;

namespace script {
class Context;
}

// Can be used to provide extra things to the opcode handlers
// without having to modify the signature
struct Context
{
    Player& player;
    script::Context& script;
};

enum Opcode : uint16_t
{
    TEST = 0,
    JUMP = 1,
    REPL = 2
};

// TODO: change this to be something more easily testable
// currently, for testing purposes, there is a literal "test" packet, which does nothing other than
// give us the ability to verify that this was called
// similar to net::Handler?
void Handle(game::Context& context, game::Opcode opcode, net::Packet& packet);

} // namespace game

#endif // SERVER_GAME_HANDLER_HPP_
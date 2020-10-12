#ifndef SERVER_GAME_HANDLER_HPP_
#define SERVER_GAME_HANDLER_HPP_

#include <cstddef>
#include <cstdint>

/// To add new Opcodes + Handlers:
/// 1. Add the opcode into the Opcode enum
/// 2. Add it as a case into the switch statement @handler.cpp
/// 3. Implement the handler for it

namespace net {

class Packet;

} // namespace net

namespace game {

class Session;
class SessionManager;

// Can be used to provide extra things to the opcode handlers
// without having to modify the signature
struct Context
{
    Session& session;
};

enum Opcode : uint16_t
{
    Test = 0,
    Jump = 1
};

void Handle(game::Context& context, game::Opcode opcode, net::Packet& packet);

} // namespace game

#endif // SERVER_GAME_HANDLER_HPP_

#include "net/handler.hpp"
#include "net/packet.hpp"
#include "net/socket.hpp"

#include "game/handler.hpp"
#include "game/player.hpp"
#include "game/script.hpp"
#include "game/world.hpp"
#include "util/log.hpp"

struct Test
{
    uint16_t value;
};
void
handle_Test(game::Context& context, Test& packet)
{
    if (!context.player.valid())
        return;

    auto socket = context.player.getSocket();
    if (!socket)
        return;

    util::log::Debug("Test", "[Session#{}] value: {}", socket->getId(), packet.value);
    socket->send(std::vector<uint8_t>{ 0, 0, 0, 0 });
    util::log::Debug("Test", "[Session#{}] responsed with [0, 0, 0, 0]", socket->getId());
}

struct Jump
{};
void
handle_Jump(game::Context& context, Jump&)
{
    if (!context.player.valid())
        return;

    auto socket = context.player.getSocket();
    if (!socket)
        return;
    util::log::Debug("Jump", "[Session#{}]", context.player.getSocket()->getId());
}

struct REPL
{
    std::string code;
};
template<>
inline bool
net::Deserialize(net::Packet& packet, REPL& repl)
{
    if (packet.remaining() < sizeof(uint16_t))
        return false;
    net::Deserialize(packet, repl.code);
    return true;
}
void
handle_REPL(game::Context& context, REPL& repl)
{
    if (!context.player.valid())
        return;

    if (auto sock = context.player.getSocket()) {
        net::Packet packet{};
        util::log::Debug("REPL", "[Session#{}]: Input = {}", sock->getId(), repl.code);
        try {
            auto result = context.script.eval(repl.code);
            if (!result.valid()) {
                net::Serialize(packet, std::string{ "Invalid result" });
                util::log::Debug("REPL", "[Session#{}]: Error = {}", sock->getId(), "Invalid result");
            } else if (result.get_type() != sol::type::string) {
                net::Serialize(packet, std::string{ "Result must be string" });
                util::log::Debug("REPL", "[Session#{}]: Error = {}", sock->getId(), "Result must be string");
            } else {
                net::Serialize(packet, result.get<std::string>());
                util::log::Debug("REPL", "[Session#{}]: Output = {}", sock->getId(), result.get<std::string>());
            }
        } catch (std::exception& e) {
            std::string error = e.what();
            net::Serialize(packet, error);
            util::log::Debug("REPL", "[Session#{}]: Error = {}", sock->getId(), error);
        }
        sock->send(packet);
    }
}

void
game::Handle(game::Context& context, game::Opcode opcode, net::Packet& packet)
{
    switch (opcode) {
        case Opcode::TEST: {
            ::Test t{};
            if (!net::Deserialize(packet, t))
                return;
            return handle_Test(context, t);
        }
        case Opcode::JUMP: {
            ::Jump j{};
            if (!net::Deserialize(packet, j))
                return;
            return handle_Jump(context, j);
        }
        case Opcode::REPL: {
            ::REPL r{};
            if (!net::Deserialize(packet, r))
                return;
            return handle_REPL(context, r);
        }
    }
}
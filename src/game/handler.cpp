
#include "net/handler.hpp"
#include "net/packet.hpp"
#include "net/socket.hpp"

#include "game/handler.hpp"
#include "game/player.hpp"
#include "game/script.hpp"
#include "game/world.hpp"
#include "util/log.hpp"

template<typename T>
void handle(game::Context& context, T& packet);

struct Test
{
    uint16_t value;
};
template<>
void
handle(game::Context& context, Test& packet)
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
template<>
void
handle(game::Context& context, Jump&)
{
    if (!context.player.valid())
        return;

    auto socket = context.player.getSocket();
    if (!socket)
        return;
    util::log::Debug("Jump", "Player {{ Session {{ id: {} }} }}", context.player.getSocket()->getId());
}

struct REPL
{
    std::string code;
};
template<>
void
handle(game::Context& context, REPL& repl)
{
    if (!context.player.valid())
        return;

    if (auto sock = context.player.getSocket()) {
        try {
            std::string result = context.script.eval(repl.code);
            net::Packet packet;
            packet.write(result);
        } catch (sol::error& e) {
            net::Packet packet;
            std::string error = e.what();
            packet.write(error);
        }
    }
}

template<typename T>
void
DeserializeAndDispatch(game::Context& context, net::Packet& packet)
{
    if (packet.size() < packet.cursor() + sizeof(T))
        return;

    T message{};
    packet.read(message);

    handle(context, message);
}

void
game::Handle(game::Context& context, game::Opcode opcode, net::Packet& packet)
{
    switch (opcode) {
        case Opcode::Test:
            return DeserializeAndDispatch<struct Test>(context, packet);
        case Opcode::Jump:
            return DeserializeAndDispatch<struct Jump>(context, packet);
        case Opcode::REPL:
            return DeserializeAndDispatch<struct REPL>(context, packet);
    }
}

#include "game/handler.hpp"
#include "game/session.hpp"
#include "net/packet.hpp"
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
    util::log::Info("Test", "[Session#{}] value: {}", context.session.getId(), packet.value);
    if (context.session.send(std::vector<uint8_t>{ 0, 0, 0, 0 })) {
        util::log::Info("Test", "[Session#{}] response success", context.session.getId());
    } else {
        util::log::Info("Test", "[Session#{}] response failure", context.session.getId());
    }
}

struct Jump
{};
template<>
void
handle(game::Context& context, Jump&)
{
    util::log::Info("Jump", "[Session#{}]", context.session.getId());
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
    }
}
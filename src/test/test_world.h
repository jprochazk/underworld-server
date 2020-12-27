#include "pch.h"

// update -> add player
// update -> player msg
// update -> remove player

#include <gtest/gtest.h>

#include "game/opcode.hpp"
#include "game/world.hpp"
#include "net/packet.hpp"
#include "net/socket.hpp"

struct TestSocket final : public net::Socket
{
    std::vector<std::vector<uint8_t>> messages;

    void
    open(http::request<http::string_body> req, std::string token) override
    {}
    void
    close() override
    {}
    void setHandler(std::shared_ptr<net::Handler>) override {}
    void
    send(std::vector<uint8_t> data) override
    {
        messages.push_back(std::move(data));
    }
    uint32_t
    getId() override
    {
        return 0;
    }
    bool
    isOpen() override
    {
        return true;
    }
};

// NOTE: these tests currently depend on the behavior of game::Handle(Opcode::Test)

TEST(World, Connect)
{
    auto world = game::CreateWorld();
    auto handler = world->getHandler();

    auto socket = std::make_shared<TestSocket>();
    handler->onOpen(0u, socket);
    world->update();
    EXPECT_EQ(world->size(), 1u);
}
TEST(World, Message)
{
    auto world = game::CreateWorld();
    auto handler = world->getHandler();

    auto socket = std::make_shared<TestSocket>();
    handler->onOpen(0u, socket);
    world->update();
    EXPECT_EQ(world->size(), 1u);

    net::Packet pkt;
    pkt.write(game::Opcode::Test);
    pkt.write(static_cast<uint16_t>(1u));

    handler->onMessage(0u, pkt.data(), pkt.size());
    world->update();

    EXPECT_EQ(socket->messages.size(), 1u);
    EXPECT_EQ(socket->messages.front().size(), 4u);
    auto expected_response_data = std::vector<uint8_t>{ 0u, 0u, 0u, 0u };
    EXPECT_EQ(socket->messages.front(), expected_response_data);

    handler->onClose(0u);
    world->update();
    EXPECT_EQ(world->size(), 0u);
}
TEST(World, Close)
{
    auto world = game::CreateWorld();
    auto handler = world->getHandler();

    auto socket = std::make_shared<TestSocket>();
    handler->onOpen(0u, socket);
    world->update();
    EXPECT_EQ(world->size(), 1u);

    handler->onClose(0u);
    world->update();
    EXPECT_EQ(world->size(), 0u);
}
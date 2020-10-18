
// update -> add player
// update -> player msg
// update -> remove player

#include <gtest/gtest.h>

#include "game/handler.hpp"
#include "game/world.hpp"
#include "net/packet.hpp"
#include "net/socket.hpp"
#include "util/log.hpp"

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
    util::log::SetLevel(util::log::Level::Error);
    auto world = game::CreateWorld(0u);
    auto handler = world->getHandler();

    auto socket = std::make_shared<TestSocket>();
    handler->onOpen(0u, socket);
    world->update();
    EXPECT_EQ(world->size(), 1u);
}
TEST(World, Message)
{
    util::log::SetLevel(util::log::Level::Error);
    auto world = game::CreateWorld(0u);
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
    util::log::SetLevel(util::log::Level::Error);
    auto world = game::CreateWorld(0u);
    auto handler = world->getHandler();

    auto socket = std::make_shared<TestSocket>();
    handler->onOpen(0u, socket);
    world->update();
    EXPECT_EQ(world->size(), 1u);

    handler->onClose(0u);
    world->update();
    EXPECT_EQ(world->size(), 0u);
}

TEST(WorldManager, Select)
{
    size_t numWorlds = 2u;
    auto wmgr = game::CreateWorldManager(numWorlds);

    for (size_t i = 0; i < numWorlds; ++i) {
        auto socket = std::make_shared<TestSocket>();
        auto handler = wmgr->select();
        handler->onOpen(0u, socket);
        wmgr->get(handler->id())->update();
    }

    for (size_t i = 0; i < numWorlds; ++i) {
        EXPECT_EQ(wmgr->get(i)->size(), 1u);
    }
}
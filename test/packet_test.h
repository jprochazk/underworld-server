
#include "net/packet.hpp"
#include <gtest/gtest.h>
#include <iostream>

struct Foo
{
    uint32_t foo;
    uint32_t bar;
    float baz;
};

TEST(Packet, Write)
{
    Foo foo{ 128592315u, 128592315u, 10.5 };

    net::Packet packet;
    packet.write(foo);

    EXPECT_EQ(packet.size(), sizeof(Foo));
}

TEST(Packet, Read)
{   
    // [0, 3]  = 128592315u
    // [4, 7]  = 128592315u
    // [8, 11] = 10.5f
    net::Packet packet{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };

    Foo foo{};
    packet.read(foo);

    EXPECT_EQ(foo.foo, 128592315u);
    EXPECT_EQ(foo.bar, 128592315u);
    EXPECT_NEAR(foo.baz, 10.5f, std::numeric_limits<float>::epsilon());
}

TEST(Packet, ReadWrite)
{
    Foo foo1{ 128592315u, 128592315u, 10.5 };

    net::Packet packet1;
    packet1.write(foo1);

    EXPECT_EQ(packet1.size(), sizeof(Foo));

    // packets aren't meant to be read and written from at the same time...
    // so just yoink the data :)
    net::Packet packet2{ packet1.data(), packet1.size() };

    Foo foo2{};
    packet2.read(foo2);

    EXPECT_EQ(foo2.foo, foo1.foo);
    EXPECT_EQ(foo2.bar, foo1.bar);
    EXPECT_NEAR(foo2.baz, foo1.baz, std::numeric_limits<float>::epsilon());
}

#include "net/packet.hpp"
#include <gtest/gtest.h>
#include <iostream>

TEST(Packet, Read_Simple)
{
    uint32_t expected = 128592315u;
    net::Packet packet{ { 0x07, 0xAA, 0x29, 0xBB } };

    uint32_t actual{};
    packet.read(actual);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Write_Simple)
{
    net::Packet expected{ { 0x07, 0xAA, 0x29, 0xBB } };
    uint32_t value = 128592315u;

    net::Packet actual;
    actual.write(value);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Read_Struct)
{   
    struct Foo
    {
        uint32_t foo;
        uint32_t bar;
        float baz;

        bool operator==(const Foo& other) {
            return foo == other.foo && bar == other.bar && baz == other.baz;
        }
    };

    Foo expected { 128592315u, 128592315u, 10.5f };
    net::Packet packet{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };

    Foo actual{};
    packet.read(actual);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Write_Struct)
{
    struct Foo
    {
        uint32_t foo;
        uint32_t bar;
        float baz;

        bool operator==(const Foo& other) {
            return foo == other.foo && bar == other.bar && baz == other.baz;
        }
    };

    net::Packet expected{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };
    Foo value{ 128592315u, 128592315u, 10.5 };

    net::Packet actual;
    actual.write(value);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Read_Vector_Simple)
{
    std::vector<uint32_t> expected = { 128592315u, 128592315u, 128592315u };
    net::Packet packet{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB } };

    std::vector<uint32_t> actual{};
    packet.read(actual, 3);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Write_Vector_Simple)
{
    net::Packet expected{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB } };
    std::vector<uint32_t> value = { 128592315u, 128592315u, 128592315u };

    net::Packet actual;
    actual.write(value);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Read_Vector_Struct)
{
    struct Foo
    {
        uint32_t foo;
        uint32_t bar;
        float baz;

        bool operator==(const Foo& other) {
            return foo == other.foo && bar == other.bar && baz == other.baz;
        }
    };

    std::vector<Foo> expected = {
        Foo { 128592315u, 128592315u, 10.5f },
        Foo { 128592315u, 128592315u, 10.5f },
        Foo { 128592315u, 128592315u, 10.5f }
    };
    net::Packet packet{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };

    std::vector<Foo> actual{};
    packet.read(actual, 3);

    for (size_t i = 0; i < actual.size(); ++i) {
        EXPECT_TRUE(actual[i] == expected[i]);
    }
}

TEST(Packet, Write_Vector_Struct)
{
    struct Foo
    {
        uint32_t foo;
        uint32_t bar;
        float baz;

        bool operator==(const Foo& other) {
            return foo == other.foo && bar == other.bar && baz == other.baz;
        }
    };

    net::Packet expected{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };
    std::vector<Foo> value = {
        Foo { 128592315u, 128592315u, 10.5f },
        Foo { 128592315u, 128592315u, 10.5f },
        Foo { 128592315u, 128592315u, 10.5f }
    };

    net::Packet actual{};
    actual.write(value);

    EXPECT_TRUE(actual == expected);
}
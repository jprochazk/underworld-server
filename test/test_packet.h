
#include "net/packet.hpp"
#include <gtest/gtest.h>
#include <iostream>

struct Foo
{
    uint32_t foo;
    uint32_t bar;
    float baz;
};

inline bool
operator==(const Foo& a, const Foo& b)
{
    return a.foo == b.foo && a.bar == b.bar && a.baz == b.baz;
}

std::ostream&
operator<<(std::ostream& os, const Foo& foo)
{
    return os << "{ "
              << "foo: " << foo.foo << ", "
              << "bar: " << foo.bar << ", "
              << "baz: " << foo.baz << "} ";
}

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
    Foo expected{ 128592315u, 128592315u, 10.5f };
    net::Packet packet{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };

    Foo actual{};
    packet.read(actual);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Write_Struct)
{
    net::Packet expected{ { 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };
    Foo value{ 128592315u, 128592315u, 10.5 };

    net::Packet actual;
    actual.write(value);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Read_Vector_Simple)
{
    std::vector<uint32_t> expected = { 128592315u, 128592315u, 128592315u };
    net::Packet packet{ { 0x00, 0x03, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB } };

    std::vector<uint32_t> actual{};
    packet.read(actual);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Write_Vector_Simple)
{
    net::Packet expected{ { 0x00, 0x03, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB } };
    std::vector<uint32_t> value = { 128592315u, 128592315u, 128592315u };

    net::Packet actual;
    actual.write(value);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Read_Vector_Struct)
{

    std::vector<Foo> expected = { Foo{ 128592315u, 128592315u, 10.5f },
                                  Foo{ 128592315u, 128592315u, 10.5f },
                                  Foo{ 128592315u, 128592315u, 10.5f } };
    net::Packet packet{ { 0x00, 0x03, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00,
                          0x00, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00,
                          0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };

    std::vector<Foo> actual{};
    uint16_t count;
    packet.read(count);
    actual.resize(count);
    for (size_t i = 0; i < count; ++i) {
        packet.read(actual[i]);
    }

    for (size_t i = 0; i < actual.size(); ++i) {
        EXPECT_EQ(actual[i], expected[i]);
    }
}

TEST(Packet, Write_Vector_Struct)
{
    net::Packet expected{ { 0x00, 0x03, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00,
                            0x00, 0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00,
                            0x07, 0xAA, 0x29, 0xBB, 0x07, 0xAA, 0x29, 0xBB, 0x41, 0x28, 0x00, 0x00 } };
    std::vector<Foo> value = { Foo{ 128592315u, 128592315u, 10.5f },
                               Foo{ 128592315u, 128592315u, 10.5f },
                               Foo{ 128592315u, 128592315u, 10.5f } };

    net::Packet actual{};
    actual.write(value);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Read_String)
{
    std::string expected = "test";
    net::Packet packet{ { 0x00, 0x04, 0x74, 0x65, 0x73, 0x74 } };

    std::string actual{};
    packet.read(actual);

    EXPECT_TRUE(actual == expected);
}

TEST(Packet, Write_String)
{
    net::Packet expected{ { 0x00, 0x04, 0x74, 0x65, 0x73, 0x74 } };
    std::string value = "test";

    net::Packet actual{};
    actual.write(value);

    for (auto& b : actual) {
        std::cout << std::to_string(b) << std::endl;
    }

    EXPECT_TRUE(actual == expected);
}
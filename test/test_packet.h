
#include "net/packet.hpp"
#include <gtest/gtest.h>

// POD-type for tests below
struct Type_POD
{
    uint32_t foo;
    uint32_t bar;
    float baz;

    bool
    operator==(const Type_POD& other) const
    {
        return foo == other.foo && bar == other.bar && baz == other.baz;
    }
};

// Non-trivial type for tests below
struct Type_NT
{
    uint32_t foo;
    std::string bar;
    std::vector<float> baz;

    bool
    operator==(const Type_NT& other) const
    {
        return foo == other.foo && bar == other.bar && baz == other.baz;
    }
};

template<>
inline bool
net::Deserialize(net::Packet& packet, Type_NT& data)
{
    if (packet.remaining() < sizeof(data.foo) + sizeof(uint16_t) + sizeof(uint16_t))
        return false;
    net::Deserialize(packet, data.foo);
    net::Deserialize(packet, data.bar);
    net::Deserialize(packet, data.baz);
    return true;
}

// TODO: debug this

TEST(Packet, Read_Arithmetic)
{
    uint32_t expected = 1234567890u;
    net::Packet input = { 0xD2, 0x02, 0x96, 0x49 };
    auto actual = uint32_t{};
    net::Deserialize(input, actual);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Write_Arithmetic)
{
    net::Packet expected = { 0xD2, 0x02, 0x96, 0x49 };
    uint32_t input = 1234567890u;
    auto actual = net::Packet{};
    net::Serialize(actual, input);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Read_POD)
{
    Type_POD expected = { 1234567890u, 1234567890u, 10.5 };
    net::Packet input = { 0xD2, 0x02, 0x96, 0x49, 0xD2, 0x02, 0x96, 0x49, 0x00, 0x00, 0x28, 0x41 };
    auto actual = Type_POD{};
    net::Deserialize(input, actual);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Write_POD)
{
    net::Packet expected = { 0xD2, 0x02, 0x96, 0x49, 0xD2, 0x02, 0x96, 0x49, 0x00, 0x00, 0x28, 0x41 };
    Type_POD input = { 1234567890u, 1234567890u, 10.5 };
    auto actual = net::Packet{};
    net::Serialize(actual, input);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Read_String)
{
    std::string expected = "test";
    net::Packet input{ { 0x04, 0x00, 0x74, 0x65, 0x73, 0x74 } };
    auto actual = std::string{};
    net::Deserialize(input, actual);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Write_String)
{
    net::Packet expected = { 0x04, 0x00, 0x74, 0x65, 0x73, 0x74 };
    std::string input = "test";
    auto actual = net::Packet{};
    net::Serialize(actual, input);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Read_Vector_Arithmetic)
{
    std::vector<uint16_t> expected = { 123u, 123u, 123u, 123u };
    net::Packet input = { 0x04, 0x00, 0x7B, 0x00, 0x7B, 0x00, 0x7B, 0x00, 0x7B, 0x00 };
    auto actual = std::vector<uint16_t>{};
    net::Deserialize(input, actual);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Write_Vector_Arithmetic)
{
    net::Packet expected = { 0x04, 0x00, 0x7B, 0x00, 0x7B, 0x00, 0x7B, 0x00, 0x7B, 0x00 };
    std::vector<uint16_t> input = { 123u, 123u, 123u, 123u };
    auto actual = net::Packet{};
    net::Serialize(actual, input);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Read_Vector_String)
{
    std::vector<std::string> expected = { "test", "test", "test", "test" };
    net::Packet input = { 0x04, 0x00, 0x04, 0x00, 0x74, 0x65, 0x73, 0x74, 0x04, 0x00, 0x74, 0x65, 0x73,
                          0x74, 0x04, 0x00, 0x74, 0x65, 0x73, 0x74, 0x04, 0x00, 0x74, 0x65, 0x73, 0x74 };
    auto actual = std::vector<std::string>{};
    net::Deserialize(input, actual);
    EXPECT_EQ(actual, expected);
}
TEST(Packet, Write_Vector_String)
{
    net::Packet expected = { 0x04, 0x00, 0x04, 0x00, 0x74, 0x65, 0x73, 0x74, 0x04, 0x00, 0x74, 0x65, 0x73,
                             0x74, 0x04, 0x00, 0x74, 0x65, 0x73, 0x74, 0x04, 0x00, 0x74, 0x65, 0x73, 0x74 };
    std::vector<std::string> input = { "test", "test", "test", "test" };
    auto actual = net::Packet{};
    net::Serialize(actual, input);
    EXPECT_EQ(actual, expected);
}
/* TODO: write these :)
TEST(Packet, Read_Non_Trivial)
{
    // Read:
    // 1. expected value
    // 2. input packet
    // 3. actual value
    // 4. assert actual == expected
}
TEST(Packet, Write_Non_Trivial)
{
    // Write:
    // 1. expected packet
    // 2. input value
    // 3. actual packet
    // 4. assert actual == expected
}
TEST(Packet, Read_Vector_Non_Trivial)
{
    // Read:
    // 1. expected value
    // 2. input packet
    // 3. actual value
    // 4. assert actual == expected
}
TEST(Packet, Write_Vector_Non_Trivial)
{

    // Write:
    // 1. expected packet
    // 2. input value
    // 3. actual packet
    // 4. assert actual == expected
}
 */
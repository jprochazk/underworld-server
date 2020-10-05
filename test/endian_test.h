
#include <gtest/gtest.h>

#include "net/endian.hpp"

TEST(Endian, Convert_Unsigned)
{
    // uint8_t
    {
        uint8_t big = 0xAF;
        uint8_t little = 0xAF;
        endian::network_order(little);
        EXPECT_EQ(big, little);
    }
    {
        uint8_t big = 0xAF;
        uint8_t little = 0xAF;
        endian::native_order(big);
        EXPECT_EQ(big, little);
    }
    // uint16_t
    {
        uint16_t big = 0xAFFA;
        uint16_t little = 0xFAAF;
        endian::network_order(little);
        EXPECT_EQ(big, little);
    }
    {
        uint16_t big = 0xAFFA;
        uint16_t little = 0xFAAF;
        endian::native_order(big);
        EXPECT_EQ(big, little);
    }
    // uint32_t
    {
        uint32_t big = 0xAFFAAFFA;
        uint32_t little = 0xFAAFFAAF;
        endian::network_order(little);
        EXPECT_EQ(big, little);
    }
    {
        uint32_t big = 0xAFFAAFFA;
        uint32_t little = 0xFAAFFAAF;
        endian::native_order(big);
        EXPECT_EQ(big, little);
    }
    // uint64_t
    {
        uint64_t big = 0xAFFAAFFAAFFAAFFA;
        uint64_t little = 0xFAAFFAAFFAAFFAAF;
        endian::network_order(little);
        EXPECT_EQ(big, little);
    }
    {
        uint64_t big = 0xAFFAAFFAAFFAAFFA;
        uint64_t little = 0xFAAFFAAFFAAFFAAF;
        endian::native_order(big);
        EXPECT_EQ(big, little);
    }
}

TEST(Endian, Convert_Float)
{
    {
        float big = -24.0000476837;
        float little = 6.65637124179e-24;
        endian::network_order(little);
        EXPECT_NEAR(big, little, 0.1);
    }
}
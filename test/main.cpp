
#include <gtest/gtest.h>

#include "test_packet.h"
#include "test_world.h"

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}